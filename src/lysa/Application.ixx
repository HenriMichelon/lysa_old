/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.application;

import std;
import lysa.application_config;

export namespace lysa {

    class Application {
    public:
        static std::shared_ptr<Application> createApplication(ApplicationConfig& applicationConfig);

        virtual ~Application() = default;
    };

};