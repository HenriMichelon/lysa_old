/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.viewport;

import vireo;
import lysa.configuration;
import lysa.exception;
import lysa.input_event;
import lysa.scene;
import lysa.types;
import lysa.nodes.camera;
import lysa.nodes.node;
import lysa.physics.engine;
import lysa.renderers.debug;
import lysa.renderers.vector;

export namespace lysa {

    /**
     * Viewport orchestrates a self‑contained rendering context.
     *  - Own a root node (scene tree) and manage per‑frame scene instances.
     *  - Handle deferred add/remove of nodes and camera activation between frames.
     *  - Bridge input and per‑frame updates to the node tree (process/physics).
     *  - Provide a vireo::Viewport and scissors used by render passes.
     *  - Drive rendering of a Scene into color/depth render targets.
     *
     * Notes:
     *  - Create with a ViewportConfiguration, then attach to a Window (friend).
     *  - Use setRootNode() to change the active scene tree.
     *  - Nodes can be added/removed immediately or deferred (async parameter).
     *  - activateCamera() schedules camera switching at a safe point.
     *  - Root node and per‑frame queues are protected by mutexes where needed.
     *  - Public mutators should be called from the main/render thread unless
     *    explicitly documented; async queues are processed on the frame boundary.
     */
    class Viewport  {
    public:
        friend class Window;
        /**
         * Constructs a viewport with the provided configuration.
         *
         * @param config Viewport configuration (dimensions, buffers, features).
         */
        Viewport(ViewportConfiguration& config);

        /**
         * Changes the current scene root node.
         *
         * The provided node must not have a parent; ownership is shared.
         *
         * @param node New root node of the scene graph.
         */
        void setRootNode(const std::shared_ptr<Node> &node);

        /**
         * Returns whether the scene is currently paused.
         * This controls Node processing with respect to ProcessMode.
         *
         * @return True if paused, false otherwise.
         */
        auto isPaused() const { return paused; }

        /**
         * Pauses or resumes the current scene.
         *
         * @param pause New state (true pauses, false resumes).
         */
        void setPaused(bool pause);

        /**
         * Schedules activation of the specified camera for the next frame.
         * The camera must belong to the current scene.
         *
         * @param camera Camera to activate.
         */
        void activateCamera(const std::shared_ptr<Camera> &camera);

        /**
         * Adds a node to the current scene.
         *
         * @param node              Node to add.
         * @param async             If true, queues the add for the next frame.
         * @param attachToViewport  If true, attaches the node under the viewport root.
         */
        void addNode(const std::shared_ptr<Node> &node, bool async, bool attachToViewport);

        /**
         * Removes a node from the current scene.
         *
         * @param node  Node to remove.
         * @param async If true, queues the removal for the next frame.
         */
        void removeNode(const std::shared_ptr<Node> &node, bool async);

        /** Returns the aspect ratio (width/height) of the viewport. */
        auto getAspectRatio() const { return viewport.width / viewport.height; }

        /**
         * Returns the low‑level viewport definition (x, y, width, height, depth).
         */
        const auto& getViewport() const { return viewport; }

        /**
         * Sets the low‑level viewport definition.
         */
        void setViewport(const vireo::Viewport& viewport) { this->viewport = viewport; }

        /**
         * Returns the scissors rectangle applied to rendering.
         */
        const auto& getScissors() const { return scissors; }

        /**
         * Sets the scissors rectangle applied to rendering.
         */
        void setScissors(const vireo::Rect& scissors) { this->scissors = scissors; }

        /**
         * Returns the Window this viewport is attached to.
         * Asserts if not attached.
         */
        auto& getWindow() const {
            assert([&]{ return window != nullptr;}, "Viewport not attached to a window");
            return *window;
        }

        /** Prevents processing of deferred add/remove updates for the next frames. */
        void lockDeferredUpdate() {
            lockDeferredUpdates = true;
        }

        /** Allows processing of deferred add/remove updates again. */
        void unlockDeferredUpdate() {
            lockDeferredUpdates = false;
        }

        /** Returns the physics scene associated with this viewport. */
        auto& getPhysicsScene() const {
            return *physicsScene;
        }

        /** Returns the debug renderer (may be null). */
        auto getDebugRenderer() const {
            return debugRenderer;
        }

        /** Returns the vector renderer (may be null). */
        auto getVectorRenderer() { return vectorRenderer; }

        /** Updates the current frame (game state and scene) prior to rendering. */
        void update(const vireo::CommandList& commandList, uint32 frameIndex) const;

        /**
         * Renders the provided Scene into the given color and depth attachments.
         *
         * @param commandList     Command buffer to record draw calls.
         * @param scene           Scene to draw (camera, lights, instances).
         * @param colorAttachment Target color render surface.
         * @param depthAttachment Target depth render surface.
         * @param frameIndex      Index of the in‑flight frame.
         */
        void draw(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            uint32 frameIndex) const;

        /** Returns the immutable configuration used to create this viewport. */
        const auto& getConfiguration() const {
            return config;
        }

        /** Returns true when debug overlays are enabled. */
        auto getDisplayDebug() const { return displayDebug;}

        /** Enables or disables debug overlays rendering. */
        void setDisplayDebug(const bool displayDebug) { this->displayDebug = displayDebug; }

        virtual ~Viewport();
        Viewport(Viewport&) = delete;
        Viewport& operator=(Viewport&) = delete;

    private:
        friend class Window;

        /** Per‑frame state and deferred operations processed at frame boundaries. */
        struct FrameData {
            /** Nodes to add on the next frame (synchronous path). */
            std::list<std::shared_ptr<Node>> addedNodes;
            /** Nodes to add on the next frame (async path). */
            std::list<std::shared_ptr<Node>> addedNodesAsync;
            /** Nodes to remove on the next frame (synchronous path). */
            std::list<std::shared_ptr<Node>> removedNodes;
            /** Nodes to remove on the next frame (async path). */
            std::list<std::shared_ptr<Node>> removedNodesAsync;
            /** True when a new camera must be activated on the next frame. */
            bool cameraChanged{false};
            /** Camera to activate on the next frame. */
            std::shared_ptr<Camera> activeCamera;
            /** Scene instance associated with this frame. */
            std::shared_ptr<Scene> scene;
        };

        /** Immutable reference to the viewport configuration. */
        ViewportConfiguration& config;
        /** Low‑level viewport (x, y, width, height, minDepth, maxDepth). */
        vireo::Viewport        viewport{};
        /** Scissors rectangle limiting rendering to a sub‑area. */
        vireo::Rect            scissors{};
        /** Window this viewport is attached to (set by Window::attach). */
        Window*                window{nullptr};
        /** Root of the node tree displayed by this viewport. */
        std::shared_ptr<Node> rootNode;
        /** Guards concurrent access to the root node. */
        std::mutex            rootNodeMutex;
        /** Array of per‑frame data (one entry per in‑flight frame). */
        std::vector<FrameData> framesData;
        /** Guards modifications to per‑frame queues. */
        std::mutex             frameDataMutex;
        /** True when scene processing is paused. */
        bool                  paused{false};
        /** Prevent processing of deferred queues while true. */
        bool                  lockDeferredUpdates{false};

        /** Physics world associated with this viewport. */
        std::unique_ptr<PhysicsScene> physicsScene;
        /** Optional renderer that draws debug primitives. */
        std::shared_ptr<DebugRenderer> debugRenderer;
        /** Optional renderer for vector graphics. */
        std::shared_ptr<VectorRenderer> vectorRenderer;
        /** Toggles debug overlays rendering. */
        bool displayDebug{false};

        /** Returns the Scene object associated with the specified frame. */
        auto& getScene(const uint32 frameIndex) const { return framesData[frameIndex].scene; }

        /** Propagates an input event to the node tree; returns true if consumed. */
        bool input(InputEvent &inputEvent) { return input(rootNode, inputEvent); }

        /** Internal recursive input dispatch to a node and its children. */
        bool input(const std::shared_ptr<Node> &node, InputEvent &inputEvent);

        /** Attaches this viewport to a window (called by Window). */
        void attachToWindow(Window& window);

        /** Advances simulation for the specified frame (deferred ops, state). */
        void update(uint32 frameIndex);

        /** Steps the physics simulation. */
        void physicsProcess(float delta) const;

        /** Updates nodes with the given alpha (interpolation factor). */
        void process(float alpha) const;

        /** Issues draw calls for the current frame. */
        void render(uint32 frameIndex) const;

        /** Applies deferred add/remove queues and camera changes. */
        void processDeferredUpdates(uint32 frameIndex);

        /** Handles viewport resize events. */
        void resize(const vireo::Extent &extent);

        /** Closes the viewport and releases resources. */
        void close();

    };

}
