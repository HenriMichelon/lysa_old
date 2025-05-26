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
import lysa.resources;
import lysa.window;

export namespace lysa {

    class Application {
    public:
        friend class Node;

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

        static auto& getInstance() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return *instance;
        }

        virtual ~Application();

    private:
        static Application* instance;
        ApplicationConfiguration& config;
        std::shared_ptr<vireo::Vireo> vireo;
        Resources resources;
        std::list<std::shared_ptr<Window>> windows;
        std::shared_ptr<vireo::SubmitQueue> graphicQueue;
        std::shared_ptr<vireo::CommandAllocator> commandAllocator;
        std::shared_ptr<vireo::CommandList> commandList;
        bool quit{false};

        // Fixed delta time for the physics
        static constexpr float FIXED_DELTA_TIME{1.0f/60.0f};
        double currentTime{0.0};
        double accumulator{0.0};

        void drawFrame();

        void mainLoop();

    };

};