/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.application;

import lysa.windows.surface;

namespace lysa {

    Application::Application(ApplicationConfig& applicationConfig):
        applicationConfig{applicationConfig} {
    }

    std::shared_ptr<Surface> Application::createSurface(SurfaceConfig& surfaceConfig, void* windowHandle) const {
#ifdef _WIN32
        return std::make_shared<WindowsSurface>(surfaceConfig, windowHandle);
#endif
    }

}