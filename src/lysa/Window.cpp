/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.window;

import lysa.application;
import lysa.global;
import lysa.nodes.node;
import lysa.renderers.forward_renderer;

namespace lysa {

    Window::Window(
        WindowConfiguration& config,
        void* windowHandle,
        const std::shared_ptr<Node>& rootNode):
        windowHandle{windowHandle},
        config{config},
        graphicQueue{Application::getVireo().createSubmitQueue(vireo::CommandType::GRAPHIC, L"Main Queue")},
        swapChain{Application::getVireo().createSwapChain(
            config.renderingConfig.renderingFormat,
            graphicQueue,
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
        }
        const auto viewport = std::make_shared<Viewport>(config.viewportConfig);
        addViewport(viewport);
        renderer = std::make_unique<ForwardRenderer>(config.renderingConfig, L"Main Renderer"); // Must be instantiated after SceneData for the layout
        renderer->resize(swapChain->getExtent());
        viewport->setRootNode(rootNode);
    }

    Window::~Window() {
        graphicQueue->waitIdle();
        swapChain->waitIdle();
        framesData.clear();
        renderer.reset();
    }

    std::shared_ptr<Viewport> Window::addViewport(const std::shared_ptr<Viewport>& viewport) {
        waitIdle();
        viewport->attachToWindow(*this);
        viewports.push_back(viewport);
        return viewport;
    }

    void Window::drawFrame() {
        const auto frameIndex = swapChain->getCurrentFrameIndex();
        for (const auto& viewport : viewports) {
            viewport->update(frameIndex);
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
            while (accumulator >= FIXED_DELTA_TIME) {
                // Update physics here
                for (const auto& viewport : viewports) {
                    viewport->physicsProcess(FIXED_DELTA_TIME);
                }
                accumulator -= FIXED_DELTA_TIME;
            }
            for (const auto& viewport : viewports) {
                viewport->process(static_cast<float>(accumulator / FIXED_DELTA_TIME));
            }
        }
        render(frameIndex);
    }

    void Window::render(const uint32 frameIndex) const {
        const auto& frame = framesData[frameIndex];
        if (!swapChain->acquire(frame.inFlightFence)) { return; }

        frame.commandAllocator->reset();
        auto& commandList = frame.commandList;
        const auto& mainViewport = viewports.front();

        commandList->begin();
        for (const auto& viewport : viewports) {
            renderer->render(
                *commandList,
                *viewport->getScene(frameIndex),
                viewport == mainViewport,
                swapChain->getCurrentFrameIndex());
        }
        renderer->postprocess(
            *commandList,
            mainViewport->getViewport(),
            mainViewport->getScissors(),
            swapChain->getCurrentFrameIndex());

        const auto colorAttachment = renderer->getColorAttachment(frameIndex);
        commandList->barrier(colorAttachment, vireo::ResourceState::UNDEFINED,vireo::ResourceState::COPY_SRC);
        commandList->barrier(swapChain, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
        commandList->copy(colorAttachment, swapChain);
        commandList->barrier(swapChain, vireo::ResourceState::COPY_DST, vireo::ResourceState::PRESENT);
        commandList->barrier(colorAttachment, vireo::ResourceState::COPY_SRC,vireo::ResourceState::UNDEFINED);
        commandList->end();
        // commandLists.push_back(commandList);
        graphicQueue->submit(
            frame.inFlightFence,
            swapChain,
            {commandList});
        swapChain->present();
        swapChain->nextFrameIndex();
    }

    void Window::resize() const {
        const auto oldExtent = swapChain->getExtent();
        swapChain->recreate();
        const auto newExtent = swapChain->getExtent();
        if (oldExtent.width != newExtent.width || oldExtent.height != newExtent.height) {
            for (const auto& viewport : viewports) {
                viewport->resize(swapChain->getExtent());
            }
            renderer->resize(newExtent);
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

}