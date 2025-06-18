/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.window;

import lysa.application;
import lysa.global;
import lysa.log;
import lysa.nodes.node;
import lysa.renderers.forward_renderer;

namespace lysa {

    Window::Window(
        WindowConfiguration& config,
        const std::shared_ptr<Node>& rootNode):
        config{config},
        windowHandle{createWindow()},
        swapChain{Application::getVireo().createSwapChain(
            config.renderingConfig.renderingFormat,
            Application::getGraphicQueue(),
            windowHandle,
            config.renderingConfig.presentMode,
            config.renderingConfig.framesInFlight)} {
        assert([&]{return config.renderingConfig.framesInFlight > 0;}, "Must have at least 1 frame in flight");
        framesData.resize(config.renderingConfig.framesInFlight);
        auto& vireo = Application::getVireo();
        for (auto& frame : framesData) {
            frame.inFlightFence = vireo.createFence(true, L"Present Fence");
            frame.commandAllocator = vireo.createCommandAllocator(vireo::CommandType::GRAPHIC);
            frame.commandList = frame.commandAllocator->createCommandList();
            frame.commandListUpdate = frame.commandAllocator->createCommandList();
        }
        renderer = std::make_unique<ForwardRenderer>(config.renderingConfig, L"Forward Renderer");
        renderer->resize(swapChain->getExtent());
        const auto viewport = std::make_shared<Viewport>(config.mainViewportConfig);
        addViewport(viewport);
        viewport->setRootNode(rootNode);
        show();
    }

    Window::~Window() {
        stopped = true;
        swapChain->waitIdle();
        framesData.clear();
        renderer.reset();
    }

    void Window::close() {
        stopped = true;
        Application::getInstance().removeWindow(this);
    }

    std::shared_ptr<Viewport> Window::addViewport(const std::shared_ptr<Viewport>& viewport) {
        waitIdle();
        viewport->attachToWindow(*this);
        viewports.push_back(viewport);
        return viewport;
    }

    void Window::input(InputEvent &inputEvent) const {
        if (stopped) { return; }
        // if (windowManager->onInput(inputEvent)) return;
        for (const auto& viewport : viewports) {
            viewport->input(inputEvent);
        }
    }

    void Window::update() const {
        if (stopped) { return; }
        const auto frameIndex = swapChain->getCurrentFrameIndex();
        for (const auto& viewport : viewports) {
            viewport->update(frameIndex);
            if (viewport->getScene(frameIndex)->isMaterialsUpdated()) {
                renderer->updatePipelines(viewport->getScene(frameIndex)->getPipelineIds());
            }
        }
    }

    void Window::physicsProcess(const float delta) const {
        if (stopped) { return; }
        for (const auto& viewport : viewports) {
            viewport->physicsProcess(delta);
        }
    }

    void Window::process(const float alpha) const {
        if (stopped) { return; }
        for (const auto& viewport : viewports) {
            viewport->process(alpha);
        }
    }

    void Window::drawFrame() {
        if (stopped) { return; }
        const auto frameIndex = swapChain->getCurrentFrameIndex();

        const auto& frame = framesData[frameIndex];
        if (!swapChain->acquire(frame.inFlightFence)) { return; }

        frame.commandAllocator->reset();
        const auto& mainViewport = viewports.front();

        frame.commandListUpdate->begin();
        for (const auto& viewport : viewports) {
            auto& scene = *viewport->getScene(frameIndex);
            renderer->update(frame.commandListUpdate, scene);
            viewport->updateDebug(*frame.commandListUpdate, frameIndex);
        }
        frame.commandListUpdate->end();

        auto& commandList = frame.commandList;
        commandList->begin();
        for (const auto& viewport : viewports) {
            auto& scene = *viewport->getScene(frameIndex);
            renderer->render(
                commandList,
                scene,
                viewport == mainViewport,
                frameIndex);
            viewport->drawDebug(
                *commandList,
                scene,
                renderer->getColorRenderTarget(frameIndex),
                renderer->getDepthRenderTarget(frameIndex),
                frameIndex);
        }
        renderer->postprocess(
            *commandList,
            mainViewport->getViewport(),
            mainViewport->getScissors(),
            swapChain->getCurrentFrameIndex());

        const auto colorAttachment = renderer->getColorImage(frameIndex);
        commandList->barrier(colorAttachment, vireo::ResourceState::UNDEFINED,vireo::ResourceState::COPY_SRC);
        commandList->barrier(swapChain, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
        commandList->copy(colorAttachment, swapChain);
        commandList->barrier(swapChain, vireo::ResourceState::COPY_DST, vireo::ResourceState::PRESENT);
        commandList->barrier(colorAttachment, vireo::ResourceState::COPY_SRC,vireo::ResourceState::UNDEFINED);
        commandList->end();
        Application::getGraphicQueue()->submit(
            frame.inFlightFence,
            swapChain,
            {frame.commandListUpdate, commandList});
        swapChain->present();
        swapChain->nextFrameIndex();
    }

    void Window::resize() {
        if (stopped) { return; }
        const auto oldExtent = swapChain->getExtent();
        swapChain->recreate();
        onResize();
        const auto newExtent = swapChain->getExtent();
        if (oldExtent.width != newExtent.width || oldExtent.height != newExtent.height) {
            for (const auto& viewport : viewports) {
                viewport->resize(swapChain->getExtent());
            }
            renderer->resize(newExtent);
        }
    }

    void Window::waitIdle() const {
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

}