/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.window;

import lysa.application;
import lysa.exception;
import lysa.global;
import lysa.log;
import lysa.nodes.node;
import lysa.resources.image;
import lysa.renderers.deferred_renderer;
import lysa.renderers.forward_renderer;
import lysa.renderers.renderpass.shadow_map_pass;

namespace lysa {

    Window::Window(
        WindowConfiguration& config,
        const std::shared_ptr<Node>& rootNode):
        config{config},
        windowHandle{createWindow()},
        swapChain{Application::getVireo().createSwapChain(
            config.renderingConfig.swapChainFormat,
            Application::getGraphicQueue(),
            windowHandle,
            config.renderingConfig.presentMode,
            config.renderingConfig.framesInFlight)},
        uiRenderer{config.renderingConfig},
        windowManager{*this, uiRenderer,config.defaultFontName, config.defaultFontScale, config.defaultTextColor},
        rootNode{rootNode} {
        assert([&]{return config.renderingConfig.framesInFlight > 0;}, "Must have at least 1 frame in flight");
        framesData.resize(config.renderingConfig.framesInFlight);
        auto& vireo = Application::getVireo();
        for (auto& frame : framesData) {
            frame.inFlightFence = vireo.createFence(true, "Present Fence");
            frame.commandAllocator = vireo.createCommandAllocator(vireo::CommandType::GRAPHIC);
            frame.computeCommandList = frame.commandAllocator->createCommandList();
            frame.preRenderCommandList = frame.commandAllocator->createCommandList();
            frame.renderCommandList = frame.commandAllocator->createCommandList();
            frame.computeSemaphore = vireo.createSemaphore(vireo::SemaphoreType::BINARY);
            frame.preRenderSemaphore = vireo.createSemaphore(vireo::SemaphoreType::BINARY);
        }
        if (config.renderingConfig.rendererType == RendererType::FORWARD) {
            renderer = std::make_unique<ForwardRenderer>(config.renderingConfig, "Forward Renderer");
        } else {
            renderer = std::make_unique<DeferredRenderer>(config.renderingConfig, "Deferrer Renderer");
        }
        const auto& frame = framesData[0];
        frame.commandAllocator->reset();
        frame.preRenderCommandList->begin();
        renderer->resize(swapChain->getExtent(), frame.preRenderCommandList);
        frame.preRenderCommandList->end();
        Application::getGraphicQueue()->submit({frame.preRenderCommandList});
        Application::getGraphicQueue()->waitIdle();
        uiRenderer.resize(swapChain->getExtent());
    }

    void Window::ready() {
        const auto viewport = std::make_shared<Viewport>(config.mainViewportConfig);
        addViewport(viewport);
        viewport->setRootNode(rootNode);
        show();
    }

    Window::~Window() {
        stopped = true;
        rootNode.reset();
        swapChain->waitIdle();
        framesData.clear();
        renderer.reset();
    }

    void Window::close() {
        stopped = true;
        for (const auto& viewport : viewports) {
            viewport->close();
        }
        Application::getInstance().removeWindow(this);
    }

    std::shared_ptr<Viewport> Window::addViewport(const std::shared_ptr<Viewport>& viewport) {
        waitIdle();
        viewport->attachToWindow(*this);
        viewports.push_back(viewport);
        return viewport;
    }

    void Window::input(InputEvent &inputEvent) {
        if (stopped) { return; }
        if (windowManager.onInput(inputEvent)) return;
        for (const auto& viewport : viewports) {
            viewport->input(inputEvent);
        }
    }

    std::shared_ptr<ui::Window> Window::add(const std::shared_ptr<ui::Window> &window) {
        return windowManager.add(window);
    }

    void Window::remove(const std::shared_ptr<ui::Window> &window) {
        windowManager.remove(window);
    }

    void Window::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) const {
        renderer->updatePipelines(pipelineIds);
    }

    void Window::update() {
        if (stopped) { return; }
        const auto frameIndex = swapChain->getCurrentFrameIndex();
        for (const auto& viewport : viewports) {
            viewport->update(frameIndex);
            const auto& scene = viewport->getScene(frameIndex);
            if (scene->isMaterialsUpdated()) {
                renderer->updatePipelines(*scene);
                scene->resetMaterialsUpdated();
            }
        }
        renderer->update(frameIndex);
        // Register UI drawing commands
        windowManager.drawFrame();
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
        const auto& mainViewport = viewports.front();
        frame.commandAllocator->reset();

        frame.computeCommandList->begin();
        for (const auto& viewport : viewports) {
            auto& scene = *viewport->getScene(frameIndex);
            renderer->compute(*frame.computeCommandList, scene, frameIndex);
        }
        frame.computeCommandList->end();
        Application::getGraphicQueue()->submit(
            vireo::WaitStage::COMPUTE_SHADER,
            frame.computeSemaphore,
            {frame.computeCommandList});

        frame.preRenderCommandList->begin();
        for (const auto& viewport : viewports) {
            auto& scene = *viewport->getScene(frameIndex);
            renderer->preRender(*frame.preRenderCommandList, scene, frameIndex);
            viewport->update(*frame.preRenderCommandList, frameIndex);
        }
        uiRenderer.update(*frame.preRenderCommandList, frameIndex);

        frame.preRenderCommandList->end();
        Application::getGraphicQueue()->submit(
            frame.computeSemaphore,
            vireo::WaitStage::VERTEX_INPUT,
            vireo::WaitStage::ALL_COMMANDS,
            frame.preRenderSemaphore,
            {frame.preRenderCommandList});

        auto& commandList = frame.renderCommandList;
        commandList->begin();
        for (const auto& viewport : viewports) {
            auto& scene = *viewport->getScene(frameIndex);
            renderer->render(
                *commandList,
                scene,
                viewport == mainViewport,
                frameIndex);
        }
        renderer->postprocess(
            *commandList,
            mainViewport->getViewport(),
            mainViewport->getScissors(),
            swapChain->getCurrentFrameIndex());

        const auto colorAttachment = renderer->getColorAttachment(frameIndex);
        const auto depthAttachment = renderer->getDepthRenderTarget(frameIndex);
        for (const auto& viewport : viewports) {
            auto& scene = *viewport->getScene(frameIndex);
            viewport->drawFrame(
                *commandList,
                scene,
                colorAttachment,
                depthAttachment,
                frameIndex);
        }

        uiRenderer.render(
            *commandList,
            colorAttachment,
            depthAttachment,
            frameIndex);

        commandList->barrier(colorAttachment, vireo::ResourceState::UNDEFINED,vireo::ResourceState::COPY_SRC);
        commandList->barrier(swapChain, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
        commandList->copy(colorAttachment->getImage(), swapChain);
        commandList->barrier(swapChain, vireo::ResourceState::COPY_DST, vireo::ResourceState::PRESENT);
        commandList->barrier(colorAttachment, vireo::ResourceState::COPY_SRC,vireo::ResourceState::UNDEFINED);
        commandList->end();
        Application::getGraphicQueue()->submit(
            frame.preRenderSemaphore,
            vireo::WaitStage::VERTEX_INPUT,
            frame.inFlightFence,
            swapChain,
            {commandList});
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
            const auto& frame = framesData[0];
            frame.commandAllocator->reset();
            frame.preRenderCommandList->begin();
            for (const auto& viewport : viewports) {
                viewport->resize(swapChain->getExtent());
            }
            renderer->resize(newExtent, frame.preRenderCommandList);
            frame.preRenderCommandList->end();
            Application::getGraphicQueue()->submit({frame.preRenderCommandList});
            Application::getGraphicQueue()->waitIdle();
        }
        uiRenderer.resize(newExtent);
    }

    void Window::waitIdle() const {
        swapChain->waitIdle();
    }

    void Window::addPostprocessing(
        const std::string& fragShaderName,
        const vireo::ImageFormat outputFormat,
        void* data, const uint32 dataSize) const {
        waitIdle();
        renderer->addPostprocessing(
            fragShaderName,
            outputFormat,
            data, dataSize);
    }

    void Window::removePostprocessing(const std::string& fragShaderName) const {
        waitIdle();
        renderer->removePostprocessing(fragShaderName);
    }

}