/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.application;

import std;
import vireo;
import lysa.global;
import lysa.window;
import lysa.configuration;

export namespace lysa {

    class Application {
    public:
        Application(ApplicationConfiguration& config);

        /**
         * Returns the global Vireo object
        */
        static auto getVireo() {
            assert([&]{ return instance != nullptr;}, "Global Application instance not set");
            return instance->vireo;
        }

    private:
        static Application* instance;
        ApplicationConfiguration& config;
        // Global Vireo object
        std::shared_ptr<vireo::Vireo> vireo;
    };

};