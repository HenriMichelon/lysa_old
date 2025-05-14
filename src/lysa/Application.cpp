/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.application;

#ifdef _WIN32
import lysa.os.win32.surface;
#endif

namespace lysa {

    Application::Application(ApplicationConfig& applicationConfig):
        applicationConfig{applicationConfig} {
    }

    std::shared_ptr<Window> Application::createSurface(WindowConfig& surfaceConfig, void* windowHandle) const {
#ifdef _WIN32
        return std::make_shared<Win32Window>(surfaceConfig, windowHandle);
#endif
    }

}