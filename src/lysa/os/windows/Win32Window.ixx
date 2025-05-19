/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
export module lysa.os.win32.surface;

import std;
import vireo;
import lysa.window;
import lysa.configuration;

export namespace lysa {

    class Win32Window : public Window {
    public:
        Win32Window(const std::shared_ptr<vireo::Vireo>& vireo,WindowConfiguration&, void* windowHandle);
    };

}