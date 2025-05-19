/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
module lysa.os.win32.surface;

namespace lysa {

    Win32Window::Win32Window(const std::shared_ptr<vireo::Vireo>& vireo, WindowConfiguration& surfaceConfig, void* windowHandle) :
        Window{vireo, surfaceConfig, windowHandle} {
    }

}