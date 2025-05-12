/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.surface;

import std;
import vireo;
import lysa.surface_config;
import lysa.renderers.renderer;

export namespace lysa {

    /*
     * %A Rendering surface
     */
    class Surface {
    public:
        /**
        * Creates a rendering surface
        */
        Surface(SurfaceConfig& surfaceConfig, void* windowHandle);

        /**
         * Prepare and draw a frame
         */
        void drawFrame();

        /**
         * Returns the opaque, os-specific, window handle
         */
        auto getWindowHandle() const { return windowHandle; }

        /**
         * Returns the associated Vireo object
         */
        auto getVireo() const { return vireo; }

        /**
         * Returns the surface configuration
         */
        auto getConfig() const { return surfaceConfig; }

        virtual ~Surface();

    private:
        // Fixed delta time for the physics
        static constexpr float FIXED_DELTA_TIME{1.0f/60.0f};

        // Per frame data
        struct FrameData {
            // frames rendering & presenting synchronization
            std::shared_ptr<vireo::Fence> inFlightFence;
            // Command allocator for the main thread
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            // Command allocator for the main thread, used for the swap chain barriers
            std::shared_ptr<vireo::CommandList> commandList;
        };

        // Opaque window handle for presenting
        void*          windowHandle;
        // Surface configuration
        SurfaceConfig& surfaceConfig;

        ////// Frame drawing loop parameters
        // Last drawFrame() start time
        double         currentTime{0.0};
        // Time accumulator to calculate the process delta time
        double         accumulator{0.0};
        // Number of frames in the last second
        uint32_t       frameCount{0};
        // Number of seconds since the last FPS update
        float          elapsedSeconds{0.0f};
        // Average calculated FPS
        uint32_t       fps{0};

        ////// Vireo objects. Keep them in order for a proper destruction order
        // Associated Vireo object
        std::shared_ptr<vireo::Vireo>      vireo;
        // Swap chain for this surface
        std::shared_ptr<vireo::SwapChain>  swapChain;
        // Submission queue used to present the swap chain
        std::shared_ptr<vireo::SubmitQueue> presentQueue;
        // Per frame data
        std::vector<FrameData>             framesData;
    };

}