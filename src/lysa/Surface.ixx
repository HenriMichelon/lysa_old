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
import lysa.nodes.node;
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

        void resize() const;

        /**
         * Prepare and draw a frame
         */
        void drawFrame();

        /**
         * Returns the opaque, os-specific, window handle
         */
        // auto getWindowHandle() const { return windowHandle; }

        /**
         * Returns the associated Vireo object
         */
        // auto getVireo() const { return vireo; }

        /**
         * Returns the surface configuration
         */
        // auto getConfig() const { return surfaceConfig; }

        /**
        * Changes the current scene
        * @param node The new scene. Must not have a parent
        */
        void setRootNode(const std::shared_ptr<Node> &node);

        void waitIdle() const;

        void addPostprocessing(const std::wstring& fragShaderName, void* data = nullptr, uint32_t dataSize = 0) const;

        void removePostprocessing(const std::wstring& fragShaderName) const;

        virtual ~Surface();

    protected:

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
            //
            std::shared_ptr<vireo::Semaphore> renderingFinishedSemaphore;
        };

        // Opaque window handle for presenting
        void*                 windowHandle;
        // Surface configuration
        SurfaceConfig&        surfaceConfig;
        // Surface scene
        std::shared_ptr<Node> rootNode;

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
        std::shared_ptr<vireo::Vireo>       vireo;
        // Submission queue used to present the swap chain
        std::shared_ptr<vireo::SubmitQueue> presentQueue;
        // Swap chain for this surface
        std::shared_ptr<vireo::SwapChain>   swapChain;
        // Per frame data
        std::vector<FrameData>              framesData;

        std::unique_ptr<Renderer>           renderer;
    };

}