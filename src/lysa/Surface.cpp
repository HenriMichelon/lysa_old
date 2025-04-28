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
    }

    void Surface::drawFrame() {
        // https://gafferongames.com/post/fix_your_timestep/
        const double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        double frameTime = newTime - currentTime;

        // Calculate the FPS
        elapsedSeconds += static_cast<float>(frameTime);
        frameCount++;
        if (elapsedSeconds >= 1.0) {
            fps            = static_cast<uint32_t>(frameCount / elapsedSeconds);
            frameCount     = 0;
            elapsedSeconds = 0;
        }

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
        // Render frame here
    }

}