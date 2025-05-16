/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.window;

import lysa.global;
import lysa.nodes.node;
import lysa.renderers.forward_renderer;

namespace lysa {

    Window::Window(WindowConfig& config, void* windowHandle):
        windowHandle{windowHandle},
        config{config},
        vireo{vireo::Vireo::create(config.backend)},
        graphicQueue{vireo->createSubmitQueue(vireo::CommandType::GRAPHIC, L"Present Queue")},
        swapChain{vireo->createSwapChain(
            config.renderingFormat,
            graphicQueue,
            windowHandle,
            config.presentMode,
            config.framesInFlight)} {
        assert([&]{return config.framesInFlight > 0;}, "Must have at least 1 frame in flight");
        framesData.resize(config.framesInFlight);
        for (auto& frame : framesData) {
            frame.inFlightFence = vireo->createFence(true, L"Present Fence");
            frame.scene = std::make_shared<SceneData>(vireo, config.framesInFlight, swapChain->getExtent());
        }
        renderer = std::make_unique<ForwardRenderer>(config, vireo, L"Main Renderer"); // Must be instanciated after SceneData for the layout
        renderer->resize(swapChain->getExtent());
        setRootNode(config.rootNode);
    }

    Window::~Window() {
        graphicQueue->waitIdle();
        swapChain->waitIdle();
        framesData.clear();
        SceneData::getDescriptorLayout().reset();
        rootNode.reset();
        config.rootNode.reset();
        renderer.reset();
    }

    void Window::drawFrame() {
        const auto frameIndex = swapChain->getCurrentFrameIndex();
        const auto& frame = framesData[frameIndex];

        // Add/removes nodes from the scene
        if (!lockDeferredUpdates) {
            processDeferredUpdates(frameIndex);
        }

        const double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
        double frameTime = newTime - currentTime;

        // Calculate the FPS
        elapsedSeconds += static_cast<float>(frameTime);
        frameCount++;
        if (elapsedSeconds >= 1.0) {
            fps = static_cast<uint32>(frameCount / elapsedSeconds);
            frameCount = 0;
            elapsedSeconds = 0;
        }

        // https://gafferongames.com/post/fix_your_timestep/
        if (frameTime > 0.25) {
            frameTime = 0.25; // Note: Max frame time to avoid spiral of death
        }
        currentTime = newTime;
        accumulator += frameTime;
        {
            auto lock = std::lock_guard(rootNodeMutex);
            while (accumulator >= FIXED_DELTA_TIME) {
                // Update physics here
                if (rootNode) {
                    rootNode->physicsProcess(FIXED_DELTA_TIME);
                }
                accumulator -= FIXED_DELTA_TIME;
            }
            if (rootNode) {
                rootNode->process(static_cast<float>(accumulator / FIXED_DELTA_TIME));
            }
        }
        frame.scene->update();
        renderer->update(frameIndex);
        render(frameIndex);
    }

    void Window::render(const uint32 frameIndex) const {
        const auto& frame = framesData[frameIndex];
        if (!swapChain->acquire(frame.inFlightFence)) { return; }

        const auto commandLists = renderer->render(
            swapChain->getCurrentFrameIndex(),
            *frame.scene);

        const auto commandList = commandLists.back();
        const auto colorAttachment = renderer->getColorAttachment(frameIndex);
        commandList->barrier(colorAttachment, vireo::ResourceState::RENDER_TARGET_COLOR,vireo::ResourceState::COPY_SRC);
        commandList->barrier(swapChain, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
        commandList->copy(colorAttachment, swapChain);
        commandList->barrier(swapChain, vireo::ResourceState::COPY_DST, vireo::ResourceState::PRESENT);
        commandList->barrier(colorAttachment, vireo::ResourceState::COPY_SRC,vireo::ResourceState::UNDEFINED);
        commandList->end();
        // commandLists.push_back(commandList);
        graphicQueue->submit(
            frame.inFlightFence,
            swapChain,
            commandLists);
        swapChain->present();
        swapChain->nextFrameIndex();
    }

    void Window::resize() const {
        const auto oldExtent = swapChain->getExtent();
        swapChain->recreate();
        const auto newExtent = swapChain->getExtent();
        if (oldExtent.width != newExtent.width || oldExtent.height != newExtent.height) {
            for (auto& frame : framesData) {
                frame.scene->resize(swapChain->getExtent());
            }
            renderer->resize(newExtent);
        }
    }

    void Window::setRootNode(const std::shared_ptr<Node> &node) {
        waitIdle();
        auto lock = std::lock_guard(rootNodeMutex);
        if (rootNode) {
            removeNode(rootNode, false);
        }
        rootNode = node;
        if (rootNode) {
            assert([&]{ return node->getParent() == nullptr && node->getWindow() == nullptr;}, "Node can't be a root node");
            addNode(rootNode, false);
            rootNode->ready(this);
        }
    }

    void Window::waitIdle() const {
        graphicQueue->waitIdle();
        swapChain->waitIdle();
    }

    void Window::addPostprocessing(const std::wstring& fragShaderName, void* data, const uint32 dataSize) const {
        waitIdle();
        renderer->addPostprocessing(fragShaderName, data, dataSize);
    }

    void Window::removePostprocessing(const std::wstring& fragShaderName) const {
        waitIdle();
        renderer->removePostprocessing(fragShaderName);
    }

    void Window::setPaused(const bool state) {
        paused = state;
        // pause(rootNode);
    }

    void Window::addNode(const std::shared_ptr<Node> &node, const bool async) {
        assert([&]{return node != nullptr;}, "Node can't be null");
        lockDeferredUpdates = true;
        {
            auto lock = std::lock_guard(frameDataMutex);
            for (auto& frame : framesData) {
                if (async) {
                    frame.addedNodesAsync.push_back(node );
                } else {
                    frame.addedNodes.push_back(node );
                }
            }
        }
        node->enterScene();
        for (const auto &child : node->getChildren()) {
            addNode(child, async);
        }
        // node->_setAddedToScene(true);
        lockDeferredUpdates = false;
    }

    void Window::removeNode(const std::shared_ptr<Node> &node, const bool async) {
        assert([&]{return node != nullptr && node->getWindow() != nullptr;},
            "Node can't be null and not attached to a window");
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
        // node->_setAddedToScene(false);
        node->exitScene();
        lockDeferredUpdates = false;
    }


    void Window::processDeferredUpdates(const uint32 frameIndex) {
        // Update renderer resources
        // sceneRenderer->preUpdateScene(currentFrame);
        // Register UI drawing commands
        // windowManager->drawFrame();
        {
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
            // Batched removes
            if (!data.removedNodesAsync.empty()) {
                auto count = 0;
                for (auto it = data.removedNodesAsync.begin(); it != data.removedNodesAsync.end();) {
                    data.scene->removeNode(*it);
                    it = data.removedNodesAsync.erase(it);
                    count += 1;
                    if (count > config.maxAsyncNodesUpdatedPerFrame) { break; }
                }
            }
            // Add to the renderer the nodes previously added to the scene tree
            // Immediate additions
            if (!data.addedNodes.empty()) {
                for (const auto &node : data.addedNodes) {
                    data.scene->addNode(node);
                }
                data.addedNodes.clear();
            }
            // Batched additions
            if (!data.addedNodesAsync.empty()) {
                auto count = 0;
                for (auto it = data.addedNodesAsync.begin(); it != data.addedNodesAsync.end();) {
                    data.scene->addNode(*it);
                    it = data.addedNodesAsync.erase(it);
                    count += 1;
                    if (count > config.maxAsyncNodesUpdatedPerFrame) { break; }
                }
            }
            // Change the current camera if needed
            if (data.cameraChanged) {
                data.scene->activateCamera(data.activeCamera);
                // if (applicationConfig.debug) {
                    // debugRenderer->activateCamera(data.activeCamera, currentFrame);
                // }
                data.activeCamera.reset();
                data.cameraChanged = false;
            }
            // Search for a camera in the scene tree if there is no current camera
            if (data.scene->getCurrentCamera() == nullptr) {
                const auto &camera = rootNode->findFirstChild<Camera>(true);
                if (camera && camera->isProcessed()) {
                    data.scene->activateCamera(camera);
                    // if (applicationConfig.debug) {
                        // debugRenderer->activateCamera(camera, currentFrame);
                    // }
                }
            }
        }
        // Update renderer resources
        // sceneRenderer->postUpdateScene(currentFrame);
    }

    void Window::upload(const std::vector<vireo::BufferUploadInfo>& infos) const {
        const auto allocator = vireo->createCommandAllocator(vireo::CommandType::GRAPHIC);
        const auto commandList = allocator->createCommandList();
        commandList->begin();
        commandList->upload(infos);
        commandList->end();
        graphicQueue->submit({commandList});
        graphicQueue->waitIdle();
    }

    void Window::upload(const std::vector<vireo::ImageUploadInfo>& infos) const {
        const auto allocator = vireo->createCommandAllocator(vireo::CommandType::GRAPHIC);
        const auto commandList = allocator->createCommandList();
        commandList->begin();
        commandList->upload(infos);
        commandList->end();
        graphicQueue->submit({commandList});
        graphicQueue->waitIdle();
    }
}