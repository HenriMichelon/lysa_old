/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.application;

#ifdef _WIN32
import lysa.os.win32.window;
#endif

namespace lysa {

    Application* Application::instance{nullptr};

    Application::Application(ApplicationConfiguration& config):
        config{config},
        vireo{vireo::Vireo::create(config.backend)} {
        assert([&]{ return instance == nullptr;}, "Global Application instance already defined");
        instance = this;
    }

    std::shared_ptr<Window> Application::createWindow(WindowConfiguration& surfaceConfig, void* windowHandle) const {
#ifdef _WIN32
        return std::make_shared<Win32Window>(vireo, surfaceConfig, windowHandle);
#endif
    }

}