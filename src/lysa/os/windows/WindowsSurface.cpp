/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include "lysa/os/windows/Libraries.h"
module lysa.windows.surface;

namespace lysa {

    WindowsSurface::WindowsSurface(SurfaceConfig&, void* windowHandle) :
        hWnd{static_cast<HWND>(windowHandle)} {

    }


}