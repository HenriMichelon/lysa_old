/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <windows.h>
module lysa.application;

namespace lysa {

    void Application::mainLoop() {
        auto msg = MSG{};
        while (!quit) {
            drawFrame();
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

}

extern int lysaMain();

int WINAPI WinMain(HINSTANCE , HINSTANCE , LPSTR , int ) {
    return lysaMain();
}
