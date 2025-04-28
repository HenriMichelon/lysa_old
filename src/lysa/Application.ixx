/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.application;

import std;
import lysa.application_config;
import lysa.surface;
import lysa.surface_config;

export namespace lysa {

    class Application {
    public:
        Application(ApplicationConfig& applicationConfig);

        std::shared_ptr<Surface> createSurface(SurfaceConfig& surfaceConfig, void* windowHandle) const;

    private:
        ApplicationConfig& applicationConfig;

    };

};