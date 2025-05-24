/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.window;

import vireo;
import lysa.global;
import lysa.configuration;
import lysa.memory;
import lysa.viewport;
import lysa.nodes.node;
import lysa.renderers.renderer;

export namespace lysa {
    /**
     * %A Rendering surface
     */
    class Window {
    public:

        virtual void onReady() {}
        virtual void onClose() {}

        void resize() const;

        /**
         * Returns the opaque, os-specific, window handle
         */
        // auto getWindowHandle() const { return windowHandle; }

        /**
         * Returns the graphic submission queue
         */
        auto getGraphicQueue() const { return graphicQueue; }

        auto getAspectRatio() const { return swapChain->getAspectRatio(); }

        auto getExtent() const { return swapChain->getExtent(); }

        auto getFramesInFlight() const { return config.renderingConfig.framesInFlight; }

        auto& getViewport() const { return *viewport; }

        void waitIdle() const;

        void addPostprocessing(const std::wstring& fragShaderName, void* data = nullptr, uint32 dataSize = 0) const;

        void removePostprocessing(const std::wstring& fragShaderName) const;

        Window(WindowConfiguration& config, void* windowHandle, const std::shared_ptr<Node>& rootNode = nullptr);

        void drawFrame();

        virtual ~Window();
        Window(Window&) = delete;
        Window& operator=(Window&) = delete;

    private:
        // Fixed delta time for the physics
        static constexpr float FIXED_DELTA_TIME{1.0f/60.0f};

        // Per frame data
        struct FrameData {
            // frames rendering & presenting synchronization
            std::shared_ptr<vireo::Fence> inFlightFence;
            std::shared_ptr<Viewport> viewport;
        };

        // Opaque window handle for presenting
        void*                 windowHandle;
        // Surface configuration
        WindowConfiguration&  config;

        // Last drawFrame() start time
        double currentTime{0.0};
        // Time accumulator to calculate the process delta time
        double accumulator{0.0};
        // Number of frames in the last second
        uint32 frameCount{0};
        // Number of seconds since the last FPS update
        float  elapsedSeconds{0.0f};
        // Average calculated FPS
        uint32 fps{0};

        // Per frame data
        std::vector<FrameData>              framesData;
        std::mutex                          frameDataMutex;
        // Submission queue used to present the swap chain
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        // Swap chain for this surface
        std::shared_ptr<vireo::SwapChain>   swapChain;
        // Scene renderer
        std::unique_ptr<Renderer>           renderer;
        std::shared_ptr<Viewport>           viewport;

        void render(uint32 frameIndex) const;

    };

}