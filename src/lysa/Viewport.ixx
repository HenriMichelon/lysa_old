/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.viewport;

import vireo;
import lysa.configuration;
import lysa.global;
import lysa.input_event;
import lysa.scene;
import lysa.nodes.camera;
import lysa.nodes.node;
import lysa.physics.engine;

export namespace lysa {

    /*
     */
    class Viewport  {
    public:
        friend class Window;
        /**
         * Creates a Viewport
         */
        Viewport(ViewportConfiguration& config);

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

        /**
         * Changes the current camera
         * @param camera the camera to activate, must be in a scene
         */
        void activateCamera(const std::shared_ptr<Camera> &camera);

        // Add a node to the current scene
        void addNode(const std::shared_ptr<Node> &node, bool async);

        // Remove a node from the current scene
        void removeNode(const std::shared_ptr<Node> &node, bool async);

        auto getAspectRatio() const { return viewport.width / viewport.height; }

        /**
         * Returns the viewport
         */
        const auto& getViewport() const { return viewport; }

        /**
         * Sets the viewport
         */
        void setViewport(const vireo::Viewport& viewport) { this->viewport = viewport; }

        /**
         * Returns the scissors rect
         */
        const auto& getScissors() const { return scissors; }

        /**
         * Sets the scissors rect
         */
        void setScissors(const vireo::Rect& scissors) { this->scissors = scissors; }

        auto& getWindow() const {
            assert([&]{ return window != nullptr;}, "Viewport not attached to a window");
            return *window;
        }

        void lockDeferredUpdate() {
            lockDeferredUpdates = true;
        }

        void unlockDeferredUpdate() {
            lockDeferredUpdates = false;
        }

        auto& getPhysicsScene() const {
            return *physicsScene;
        }

        virtual ~Viewport();
        Viewport(Viewport&) = delete;
        Viewport& operator=(Viewport&) = delete;

    private:
        friend class Window;

        // Per frame data
        struct FrameData {
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
            std::shared_ptr<Scene> scene;
        };

        ViewportConfiguration& config;
        vireo::Viewport        viewport{};
        vireo::Rect            scissors{};
        Window*                window{nullptr};
        // Node tree
        std::shared_ptr<Node> rootNode;
        std::mutex            rootNodeMutex;
        // Per frame data
        std::vector<FrameData> framesData;
        std::mutex             frameDataMutex;
        // State of the current scene
        bool                  paused{false};
        //
        bool                  lockDeferredUpdates{false};

        std::unique_ptr<PhysicsScene> physicsScene;

        auto& getScene(const uint32 frameIndex) const { return framesData[frameIndex].scene; }

        bool input(InputEvent &inputEvent) { return input(rootNode, inputEvent); }

        bool input(const std::shared_ptr<Node> &node, InputEvent &inputEvent);

        void pause(const std::shared_ptr<Node> &node);

        void attachToWindow(Window& window);

        void update(uint32 frameIndex);

        void physicsProcess(float delta) const;

        void process(float alpha) const;

        void render(uint32 frameIndex) const;

        void processDeferredUpdates(uint32 frameIndex);

        void resize(const vireo::Extent &extent);

    };

}
