/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <windows.h>
module lysa.application;

using namespace std::chrono;

namespace lysa {

    void Application::mainLoop() {
        std::vector<double> frameTimes;
        auto lastTimeReport = steady_clock::now();
        auto msg = MSG{};
        while (!exit) {
            auto startTime = steady_clock::now();
            drawFrame();

            const auto endTime = steady_clock::now();
            frameTimes.push_back(duration<double, std::milli>(endTime - startTime).count());

            if (steady_clock::now() - lastTimeReport >= 1s) {
                const double avgMs = accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
                INFO("Average frame time ", avgMs, " ms");
                frameTimes.clear();
                lastTimeReport = steady_clock::now();
            }

            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

}

#ifndef LYSA_CONSOLE

extern int lysaMain();

int WINAPI WinMain(HINSTANCE , HINSTANCE , LPSTR , int ) {
    return lysaMain();
}

#endif
