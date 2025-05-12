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
        /**
         * Creates a Lysa application
         * @param applicationConfig global application configuration
         */
        Application(ApplicationConfig& applicationConfig);

        /**
         * Creates a rendering surface for a window
         * @param surfaceConfig The configuration parameter for this surface
         * @param windowHandle The opaque, os-specific, window handle
         */
        std::shared_ptr<Surface> createSurface(SurfaceConfig& surfaceConfig, void* windowHandle) const;

        /**
         * Return the global application configuration
         */
        auto getConfig() const { return applicationConfig; }

    private:
        // Global application configuration
        ApplicationConfig& applicationConfig;
    };

};