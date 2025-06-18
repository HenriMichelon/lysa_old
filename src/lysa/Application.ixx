/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.application;

import std;
import vireo;
import lysa.configuration;
import lysa.global;
import lysa.log;
import lysa.resources;
import lysa.window;
import lysa.physics.engine;

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

        virtual ~Application();

    private:
        static Application* instance;
        ApplicationConfiguration& config;
        std::shared_ptr<vireo::Vireo> vireo;
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        std::shared_ptr<vireo::SubmitQueue> computeQueue;
        Resources resources;
        std::list<std::shared_ptr<Window>> windows;
        bool quit{false};
        std::shared_ptr<Log> log;
        std::list<std::function<void()>> deferredCalls;
        std::mutex deferredCallsMutex;
        std::unique_ptr<PhysicsEngine> physicsEngine;

        // Fixed delta time for the physics
        double currentTime{0.0};
        double accumulator{0.0};

        void drawFrame();

        void mainLoop();

    };

};