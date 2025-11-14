/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.application;

import vireo;
import lysa.configuration;
import lysa.exception;
import lysa.log;
import lysa.types;
import lysa.resources;
import lysa.transfer_queue;
import lysa.window;
import lysa.physics.engine;
import lysa.resources.material;

export namespace lysa {

    /**
     * Central runtime object that owns the engine subsystems (windowing, GPU queues,
     * resources, physics, and async helpers) and drives the main loop.
     */
    class Application {
    public:
        friend class Node;
        /** Fixed time step used by the physics update loop (in seconds). */
        static constexpr float FIXED_DELTA_TIME{1.0f/60.0f};

        /** Construct the application runtime using the provided configuration. */
        Application(ApplicationConfiguration& config);

        /** Called once after initialization is complete and the engine is ready. */
        virtual void onReady() {}

        /** Called when the application is about to quit. */
        virtual void onQuit() {}

        /** Adds a window to be managed and rendered by the application. */
        void addWindow(const std::shared_ptr<Window>& window);

        /** Removes a previously added window from the application. */
        void removeWindow(Window* window);

        /** Returns a reference to the main window (the first added window). */
        auto& getMainWindow() const { return *windows.front(); }

        /** Updates rendering pipelines associated with the specified IDs. */
        void updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) const;

        /** Enter the main loop and run until quit() is requested. */
        void run();

        /**
         * Returns the global Vireo object
        */
        static const vireo::Vireo& getVireo() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return *(instance->vireo);
        }

        /** Returns the global resources manager owned by the application. */
        static Resources& getResources() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->resources;
        }

        /** Request the application to exit its main loop at the next opportunity. */
        static bool& quit() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->exit = true;
        }

        /** Returns the graphics submit queue used for rendering work. */
        static auto& getGraphicQueue() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->graphicQueue;
        }

        /** Returns the compute submit queue used for compute workloads. */
        static auto& getComputeQueue() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->computeQueue;
        }

        /** Returns the singleton application instance. */
        static Application& getInstance() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return *instance;
        }

        /** Returns the application configuration provided at startup. */
        static ApplicationConfiguration& getConfiguration() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->config;
        }

        /** Returns the physics engine owned by the application. */
        static PhysicsEngine& getPhysicsEngine() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return *instance->physicsEngine;
        }

        /** Returns the asynchronous queue helper used for background GPU work. */
        static AsyncQueue& getAsyncQueue() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->asyncQueue;
        }

        /**
        * Add a lambda expression in the deferred calls queue.<br>
        * They will be called before the next frame, after the scene pre-drawing updates
        * where nodes are added/removed from the drawing lists (for all the frames in flight).
        */
        template<typename Lambda>
        static auto callDeferred(Lambda lambda) {
            auto lock = std::lock_guard(instance->deferredCallsMutex);
            instance->deferredCalls.push_back(lambda);
        }

        /**
         * Starts a new thread that need access the GPU/VRAM.<br>
         * Use this instead of starting a thread manually because the rendering system needs
         * to wait for all the threads completion before releasing resources.
         */
        template <typename Lambda>
        static auto callAsync(Lambda lambda) {
            instance->threadedCalls.push_back(std::jthread(lambda));
        }

        virtual ~Application();

    private:
        // Singleton instance pointer set during construction and cleared on destruction.
        static Application* instance;
        /// Reference to the application configuration provided at startup.
        ApplicationConfiguration& config;
        // Backend object owning the device/instance and factory for GPU resources.
        std::shared_ptr<vireo::Vireo> vireo;
        // Submit queue used for graphics/rendering work.
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        // Submit queue used for compute workloads.
        std::shared_ptr<vireo::SubmitQueue> computeQueue;
        // Submit queue used for transfer/copy operations (uploads, staging, etc.).
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        // Global resources manager (textures, meshes, materials, shaders, ...).
        Resources resources;
        // Background submission helper for simple asynchronous GPU work.
        AsyncQueue asyncQueue;
        // Managed windows owned by the application (first is considered the main window).
        std::list<std::shared_ptr<Window>> windows;
        // Flag set to request exit from the main loop.
        bool exit{false};
        // Logging facility used by the application and subsystems.
        std::shared_ptr<Log> log;
        // Callbacks to be executed before the next frame (deferred to a safe point).
        std::list<std::function<void()>> deferredCalls;
        // Synchronizes access to the deferred calls list.
        std::mutex deferredCallsMutex;
        // Background threads launched via callAsync that must finish before shutdown.
        std::list<std::jthread> threadedCalls;
        // Synchronizes access to the threadedCalls list.
        std::mutex threadedCallsMutex;
        // Physics engine instance owned by the application.
        std::unique_ptr<PhysicsEngine> physicsEngine;

        // Fixed delta time bookkeeping for the physics update loop
        double currentTime{0.0};
        double accumulator{0.0};

        // Records and presents a single frame for all active windows.
        void drawFrame();

        // Drives the per-frame update, physics stepping, and rendering.
        void mainLoop();

        // Registers all engine/node types used by the runtime and editor.
        void registerTypes() const;
    };

};