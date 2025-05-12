/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.surface;

import lysa.renderers.forward_renderer;

namespace lysa {

    Surface::Surface(SurfaceConfig& surfaceConfig, void* windowHandle):
        windowHandle{windowHandle},
        surfaceConfig{surfaceConfig} {
        vireo = vireo::Vireo::create(surfaceConfig.backend);
        presentQueue = vireo->createSubmitQueue(vireo::CommandType::GRAPHIC, L"Present Queue");
        swapChain = vireo->createSwapChain(
            surfaceConfig.renderingFormat,
            presentQueue,
            windowHandle,
            surfaceConfig.presentMode,
            surfaceConfig.framesInFlight);
        framesData.resize(surfaceConfig.framesInFlight);
        for (auto& frame : framesData) {
            frame.inFlightFence = vireo->createFence(true, L"Present Fence");
            frame.commandAllocator = vireo->createCommandAllocator(vireo::CommandType::GRAPHIC);
            frame.commandList = frame.commandAllocator->createCommandList();
            frame.renderingFinishedSemaphore = vireo->createSemaphore(vireo::SemaphoreType::BINARY);
        }
        renderer = std::make_shared<ForwardRenderer>(surfaceConfig, vireo, L"Main Renderer");
        renderer->resize(swapChain->getExtent());
    }

    void Surface::drawFrame() {
        const double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
        double frameTime = newTime - currentTime;

        // Calculate the FPS
        elapsedSeconds += static_cast<float>(frameTime);
        frameCount++;
        if (elapsedSeconds >= 1.0) {
            fps = static_cast<uint32_t>(frameCount / elapsedSeconds);
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
                accumulator -= FIXED_DELTA_TIME;
            }
            // Process nodes here
        }

        const auto frameIndex = swapChain->getCurrentFrameIndex();
        auto& frame = framesData[frameIndex];
        if (!swapChain->acquire(frame.inFlightFence)) { return; }

        renderer->render(
            swapChain->getCurrentFrameIndex(),
            swapChain->getExtent(),
            frame.renderingFinishedSemaphore);

        const auto colorAttachment = renderer->getColorAttachment(frameIndex);
        frame.commandAllocator->reset();
        const auto commandList = frame.commandList;

        commandList->begin();
        commandList->barrier(colorAttachment, vireo::ResourceState::RENDER_TARGET_COLOR,vireo::ResourceState::COPY_SRC);
        commandList->barrier(swapChain, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
        commandList->copy(colorAttachment, swapChain);
        commandList->barrier(swapChain, vireo::ResourceState::COPY_DST, vireo::ResourceState::PRESENT);
        commandList->barrier(colorAttachment, vireo::ResourceState::COPY_SRC,vireo::ResourceState::UNDEFINED);
        commandList->end();
        presentQueue->submit(
            frame.renderingFinishedSemaphore,
            vireo::WaitStage::TRANSFER,
            frame.inFlightFence,
            swapChain,
            {frame.commandList});
        swapChain->present();
        swapChain->nextFrameIndex();
    }

    void Surface::resize() {
        const auto oldExtent = swapChain->getExtent();
        swapChain->recreate();
        const auto newExtent = swapChain->getExtent();
        if (oldExtent.width != newExtent.width || oldExtent.height != newExtent.height) {
            renderer->resize(newExtent);
        }
    }

    Surface::~Surface() {
        presentQueue->waitIdle();
        swapChain->waitIdle();
    }

}