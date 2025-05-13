/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
export module lysa.os.windows.surface;

import std;
import lysa.surface;
import lysa.surface_config;

export namespace lysa {

    class WindowsSurface : public Surface {
    public:
        WindowsSurface(SurfaceConfig&, void* windowHandle);
    };

}