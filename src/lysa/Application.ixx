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

    class Application {
    public:
        friend class Node;
        static constexpr float FIXED_DELTA_TIME{1.0f/60.0f};

        Application(ApplicationConfiguration& config);

        virtual void onReady() {}
        virtual void onQuit() {}

        void addWindow(const std::shared_ptr<Window>& window);

        void removeWindow(Window* window);

        auto& getMainWindow() const { return *windows.front(); }

        void updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) const;

        void run();

        /**
         * Returns the global Vireo object
        */
        static const auto& getVireo() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return *(instance->vireo);
        }

        static auto& getResources() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->resources;
        }

        static auto& getGraphicQueue() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->graphicQueue;
        }

        static auto& getComputeQueue() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->computeQueue;
        }

        static auto& getInstance() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return *instance;
        }

        static auto& getConfiguration() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->config;
        }

        static auto& getPhysicsEngine() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return *instance->physicsEngine;
        }

        static auto& getAsyncQueue() {
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
        static Application* instance;
        ApplicationConfiguration& config;
        std::shared_ptr<vireo::Vireo> vireo;
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        std::shared_ptr<vireo::SubmitQueue> computeQueue;
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        Resources resources;
        AsyncQueue asyncQueue;
        std::list<std::shared_ptr<Window>> windows;
        bool quit{false};
        std::shared_ptr<Log> log;
        std::list<std::function<void()>> deferredCalls;
        std::mutex deferredCallsMutex;
        std::list<std::jthread> threadedCalls;
        std::mutex threadedCallsMutex;
        std::unique_ptr<PhysicsEngine> physicsEngine;

        // Fixed delta time for the physics
        double currentTime{0.0};
        double accumulator{0.0};

        void drawFrame();

        void mainLoop();

        // Registers all nodes types
        void registerTypes() const;
    };

};