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
import lysa.nodes.camera;
import lysa.nodes.node;
import lysa.renderers.renderer;
import lysa.renderers.scene_data;

export namespace lysa {
    /**
     * %A Rendering surface
     */
    class Window {
    public:
        /**
        * Creates a rendering surface
        */
        Window(const std::shared_ptr<vireo::Vireo>& vireo, WindowConfiguration& config, void* windowHandle);

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
        auto getVireo() const { return vireo; }

        /**
         * Returns the graphic submission queue
         */
        auto getGraphicQueue() const { return graphicQueue; }

        /**
         * Returns the surface configuration
         */
        // auto getConfig() const { return surfaceConfig; }

        /**
        * Changes the current scene
        * @param node The new scene. Must not have a parent
        */
        void setRootNode(const std::shared_ptr<Node> &node);

        /**
         * Checks if the scene is paused, in respect for z0::ProcessMode
         * @return  true if the current scene is paused
         */
        auto isPaused() const { return paused; }

        /**
         * Pause or resume the current scene
         * @param pause the new state
         *  - true pause the scene
         *  - false resume the scene
         */
        void setPaused(bool pause);

        auto getAspectRatio() const { return swapChain->getAspectRatio(); }

        auto getFramesInFlight() const { return config.renderingConfig.framesInFlight; }

        void waitIdle() const;

        void addPostprocessing(const std::wstring& fragShaderName, void* data = nullptr, uint32 dataSize = 0) const;

        void removePostprocessing(const std::wstring& fragShaderName) const;

        // Add a node to the current scene
        void addNode(const std::shared_ptr<Node> &node, bool async);

        // Remove a node from the current scene
        void removeNode(const std::shared_ptr<Node> &node, bool async);

        void upload(const std::vector<vireo::BufferUploadInfo>& infos) const;

        void upload(const std::vector<vireo::ImageUploadInfo>& infos) const;

        void upload(MemoryArray& memoryArray) const;

        virtual ~Window();

    private:
        // Fixed delta time for the physics
        static constexpr float FIXED_DELTA_TIME{1.0f/60.0f};

        // Per frame data
        struct FrameData {
            // frames rendering & presenting synchronization
            std::shared_ptr<vireo::Fence> inFlightFence;
            // Deferred list of nodes added to the current scene, processed before each frame
            std::list<std::shared_ptr<Node>> addedNodes;
            std::list<std::shared_ptr<Node>> addedNodesAsync;
            // Deferred list of nodes removed from the current scene, processed before each frame
            std::list<std::shared_ptr<Node>> removedNodes;
            std::list<std::shared_ptr<Node>> removedNodesAsync;
            // Need to activate a new camera on the next frame
            bool cameraChanged{false};
            // Camera to activate next frame
            std::shared_ptr<Camera> activeCamera;
            // Scene data for the frame
            std::shared_ptr<SceneData> scene;
        };

        // Opaque window handle for presenting
        void*                 windowHandle;
        // Surface configuration
        WindowConfiguration&         config;
        // Node tree
        std::shared_ptr<Node> rootNode;
        std::mutex            rootNodeMutex;
        // State of the current scene
        bool                  paused{false};
        //
        bool                  lockDeferredUpdates{false};

        // Last drawFrame() start time
        double          currentTime{0.0};
        // Time accumulator to calculate the process delta time
        double          accumulator{0.0};
        // Number of frames in the last second
        uint32        frameCount{0};
        // Number of seconds since the last FPS update
        float           elapsedSeconds{0.0f};
        // Average calculated FPS
        uint32        fps{0};

        // Per frame data
        std::vector<FrameData>              framesData;
        std::mutex                          frameDataMutex;
        // Associated Vireo object
        std::shared_ptr<vireo::Vireo>       vireo;
        // Submission queue used to present the swap chain
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        // Swap chain for this surface
        std::shared_ptr<vireo::SwapChain>   swapChain;

        // Scene renderer
        std::unique_ptr<Renderer>           renderer;

        void render(uint32 frameIndex) const;

        // Process scene updates before drawing a frame
        void processDeferredUpdates(uint32 frameIndex);

    };

}