/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.application;

import lysa.global;
import lysa.window;
import lysa.configuration;

export namespace lysa {

    class Application {
    public:
        /**
         * Creates a rendering surface for a window
         * @param surfaceConfig The configuration parameter for this surface
         * @param windowHandle The opaque, os-specific, window handle
         */
        std::shared_ptr<Window> createWindow(WindowConfiguration& surfaceConfig, void* windowHandle) const;
    };

};