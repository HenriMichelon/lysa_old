/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.viewport;

import lysa.application;
import lysa.window;
import lysa.nodes.node;

namespace lysa {
    
    Viewport::Viewport(ViewportConfiguration& config) :
        config{config},
        viewport{config.viewport},
        scissors{config.scissors},
        physicsScene{Application::getPhysicsEngine().createScene(config.debugConfig)} {
    }

    Viewport::~Viewport() {
        lockDeferredUpdates = true;
        framesData.clear();
        rootNode.reset();
    }

    void Viewport::attachToWindow(Window& window) {
        assert([&]{ return this->window == nullptr; }, "Viewport already attached to a window");
        this->window = &window;
        auto framesInFlight = window.getFramesInFlight();
        framesData.resize(framesInFlight);
        for (auto& frame : framesData) {
            frame.scene = std::make_shared<Scene>(
                config.sceneConfig,
                window.getConfiguration().renderingConfig,
                framesInFlight,
                viewport,
                scissors);
        }
        resize(window.getExtent());
        if (config.debugConfig.enabled) {
            debugRenderer = std::make_shared<DebugRenderer>(
                config.debugConfig,
                window.getConfiguration().renderingConfig);
            displayDebug = config.debugConfig.displayAtStartup;
        }
    }

    void Viewport::resize(const vireo::Extent &extent) {
        if (config.viewport.width == 0.0f || config.viewport.height == 0.0f) {
            viewport = vireo::Viewport{
                .width = static_cast<float>(extent.width),
                .height = static_cast<float>(extent.height)
            };
        }
        if (config.scissors.width == 0.0f || config.scissors.height == 0.0f) {
            scissors = vireo::Rect{
                .x = static_cast<int32>(viewport.x),
                .y = static_cast<int32>(viewport.y),
                .width = static_cast<uint32>(viewport.width),
                .height = static_cast<uint32>(viewport.height)
            };
        }
    }

    void Viewport::update(const uint32 frameIndex) {
        if (rootNode && !lockDeferredUpdates) {
            processDeferredUpdates(frameIndex);
        }
    }

    void Viewport::updateDebug(const vireo::CommandList& commandList, const uint32 frameIndex)  const {
        if (displayDebug) {
            if (config.debugConfig.drawRayCast) {
                debugRenderer->drawRayCasts(
                    rootNode,
                    config.debugConfig.rayCastColor,
                    config.debugConfig.rayCastCollidingColor);
            }
            physicsScene->debug(*debugRenderer);
            debugRenderer->update(commandList, frameIndex);
        }
    }

    void Viewport::drawDebug(
        vireo::CommandList& commandList,
        const Scene& scene,
        std::shared_ptr<vireo::RenderTarget> colorAttachment,
        std::shared_ptr<vireo::RenderTarget> depthAttachment,
        const uint32 frameIndex)  const {
        if (displayDebug) {
            debugRenderer->render(
                    commandList,
                    scene,
                    colorAttachment,
                    depthAttachment,
                    frameIndex);
        }
    }

    void Viewport::physicsProcess(const float delta) const {
        if (rootNode) {
            if (displayDebug) {
                debugRenderer->restart();
            }
            physicsScene->update(delta);
            rootNode->physicsProcess(delta);
        }
    }

    void Viewport::process(const float alpha) const {
        if (rootNode) {
            rootNode->process(alpha);
        }
    }

    void Viewport::activateCamera(const std::shared_ptr<Camera> &camera) {
        lockDeferredUpdates = true;
        for (int i = 0; i < framesData.size(); i++) {
            framesData[i].activeCamera = camera;
            framesData[i].cameraChanged = true;
        }
        lockDeferredUpdates = false;
    }

    void Viewport::addNode(const std::shared_ptr<Node> &node, const bool async, const bool attachToViewport) {
        assert([&]{return node != nullptr;}, "Node can't be null");
        lockDeferredUpdates = true;
        {
            auto lock = std::lock_guard(frameDataMutex);
            for (auto& frame : framesData) {
                if (async) {
                    frame.addedNodesAsync.push_back(node);
                } else {
                    frame.addedNodes.push_back(node );
                }
            }
        }
        lockDeferredUpdates = false;
        for (const auto &child : node->getChildren()) {
            addNode(child, async, false);
        }
        if (attachToViewport) {
            node->attachToViewport(this);
            node->enterScene();
        }
    }

    void Viewport::removeNode(const std::shared_ptr<Node> &node, const bool async) {
        assert([&]{return node != nullptr && node->getViewport() != nullptr;},
            "Node can't be null and not attached to a viewport");
        lockDeferredUpdates = true;
        for (auto &child : node->getChildren()) {
            removeNode(child, async);
        }
        {
            auto lock = std::lock_guard(frameDataMutex);
            for (auto& frame : framesData) {
                if (async) {
                    frame.removedNodesAsync.push_back(node);
                } else {
                    frame.removedNodes.push_back(node);
                }
            }
        }
        node->detachFromViewport();
        node->exitScene();
        lockDeferredUpdates = false;
    }

    void Viewport::close() {
        if (rootNode) {
            removeNode(rootNode, false);
        }
    }

    void Viewport::processDeferredUpdates(const uint32 frameIndex) {
        auto lock = std::lock_guard(frameDataMutex);
        auto &data = framesData[frameIndex];
        // Remove from the renderer the nodes previously removed from the scene tree
        // Immediate removes
        if (!data.removedNodes.empty()) {
            for (const auto &node : data.removedNodes) {
                data.scene->removeNode(node);
            }
            data.removedNodes.clear();
        }
        // Async removes
        if (!data.removedNodesAsync.empty()) {
            auto count = 0;
            for (auto it = data.removedNodesAsync.begin(); it != data.removedNodesAsync.end();) {
                data.scene->removeNode(*it);
                it = data.removedNodesAsync.erase(it);
                count += 1;
                if (count > config.sceneConfig.maxAsyncNodesUpdatedPerFrame) { break; }
            }
        }
        // Add to the scene the nodes previously added to the scene tree
        // Immediate additions
        if (!data.addedNodes.empty()) {
            for (const auto &node : data.addedNodes) {
                data.scene->addNode(node);
            }
            data.addedNodes.clear();
        }
        // Async additions
        if (!data.addedNodesAsync.empty()) {
            auto count = 0;
            for (auto it = data.addedNodesAsync.begin(); it != data.addedNodesAsync.end();) {
                data.scene->addNode(*it);
                it = data.addedNodesAsync.erase(it);
                count += 1;
                if (count > config.sceneConfig.maxAsyncNodesUpdatedPerFrame) { break; }
            }
        }
        // Change the current camera if needed
        if (data.cameraChanged) {
            data.scene->activateCamera(data.activeCamera);
            data.activeCamera.reset();
            data.cameraChanged = false;
        }
        // Search for a camera in the scene tree if there is no current camera
        if (data.scene->getCurrentCamera() == nullptr) {
            const auto &camera = rootNode->findFirstChild<Camera>(true);
            if (camera && camera->isProcessed()) {
                data.scene->activateCamera(camera);
            }
        }
    }
    
    void Viewport::setRootNode(const std::shared_ptr<Node> &node) {
        getWindow().waitIdle();
        auto lock = std::lock_guard(rootNodeMutex);
        if (rootNode) {
            removeNode(rootNode, false);
        }
        rootNode = node;
        if (rootNode) {
            assert([&]{ return node->getParent() == nullptr && node->getViewport() == nullptr;}, "Node can't be a root node");
            addNode(rootNode, false, true);
            rootNode->ready();
        }
    }

    void Viewport::setPaused(const bool isPaused) {
        paused = isPaused;
        if (paused) {
            rootNode->pause();
        } else {
            rootNode->resume();
        }
    }

    bool Viewport::input(const std::shared_ptr<Node> &node, InputEvent &inputEvent) {
        assert([&]{ return node != nullptr; }, "Invalid node");
        for (auto &child : node->getChildren()) {
            if (input(child, inputEvent))
                return true;
        }
        if (node->isProcessed()) {
            return node->onInput(inputEvent);
        }
        return false;
    }

}
