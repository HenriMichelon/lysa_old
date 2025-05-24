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

        /**
         * Prepare and draw a frame
         */
        void drawFrame() const;

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

        virtual ~Application();

    private:
        static Application* instance;
        ApplicationConfiguration& config;
        std::shared_ptr<vireo::Vireo> vireo;
        Resources resources;
        std::vector<std::shared_ptr<Window>> windows;

    };

};