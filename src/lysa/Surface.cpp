/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.surface;

namespace lysa {

    Surface::Surface(SurfaceConfig& surfaceConfig, void* windowHandle):
        windowHandle{windowHandle},
        surfaceConfig{surfaceConfig} {
        vireo = vireo::Vireo::create(surfaceConfig.backend);
        presentQueue = vireo->createSubmitQueue(vireo::CommandType::GRAPHIC, L"PresentQueue");
        swapChain = vireo->createSwapChain(
            surfaceConfig.swapChainFormat,
            presentQueue,
            windowHandle,
            surfaceConfig.presentMode,
            surfaceConfig.framesInFlight);
        framesData.resize(surfaceConfig.framesInFlight);
        for (auto& frameData : framesData) {
            frameData.inFlightFence = vireo->createFence(true);
            frameData.commandAllocator = vireo->createCommandAllocator(vireo::CommandType::GRAPHIC);
            frameData.commandList = frameData.commandAllocator->createCommandList();
        }
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
            while (accumulator >= dt) {
                // Update physics here
                accumulator -= dt;
            }
            // Process nodes here
        }

        auto& frame = framesData[swapChain->getCurrentFrameIndex()];
        if (!swapChain->acquire(frame.inFlightFence)) { return; }
        frame.commandAllocator->reset();
        frame.commandList->begin();
        frame.commandList->barrier(swapChain, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
        frame.commandList->barrier(swapChain, vireo::ResourceState::COPY_DST, vireo::ResourceState::PRESENT);
        frame.commandList->end();
        presentQueue->submit(frame.inFlightFence, swapChain, {frame.commandList});
        swapChain->present();
        swapChain->nextFrameIndex();
    }

    Surface::~Surface() {
        swapChain->waitIdle();
    }

}