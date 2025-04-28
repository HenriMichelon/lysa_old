/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "lysa/os/windows/Libraries.h"
export module lysa.windows.surface;

import std;
import lysa.surface;
import lysa.surface_config;

export namespace lysa {

    /*
     * %A Rendering surface
     */
    class WindowsSurface : public Surface {
    public:
        WindowsSurface(SurfaceConfig&, void* windowHandle);
    private:
        HWND hWnd;
    };

}