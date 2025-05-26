/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <windows.h>
module lysa.window;

import lysa.application;

namespace lysa {

    void Window::show() const {
        ShowWindow(static_cast<HWND>(windowHandle), SW_SHOW);
    }

    void* Window::createWindow() {
        const auto hInstance = GetModuleHandle(nullptr);

        const auto windowClass = WNDCLASSEX {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = windowProcedure,
            .hInstance = hInstance,
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .lpszClassName = L"LysaGameWindowClass",
        };
        RegisterClassEx(&windowClass);

        int x = config.x;
        int y = config.y;
        int w = config.width;
        int h = config.height;
        DWORD style{0};
        DWORD exStyle{0};
        if (config.mode == WindowMode::WINDOWED) {
            if (w == 0 || h == 0 || config.mode == WindowMode::WINDOWED_MAXIMIZED) {
                exStyle = WS_EX_APPWINDOW;
                style = WS_POPUP | WS_MAXIMIZE;
                auto monitorRect = RECT{};
                const auto hPrimary = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
                auto monitorInfo = MONITORINFOEX{};
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                if (GetMonitorInfo(hPrimary, &monitorInfo)) {
                    monitorRect = monitorInfo.rcMonitor;
                } else {
                    auto monitorData = MonitorEnumData {};
                    EnumDisplayMonitors(nullptr, nullptr, monitorEnumProc, reinterpret_cast<LPARAM>(&monitorData));
                    monitorRect = monitorData.monitorRect;
                }
                w = monitorRect.right - monitorRect.left;
                h = monitorRect.bottom - monitorRect.top;
                x = monitorRect.left;
                y = monitorRect.top;
            } else {
                style = WS_OVERLAPPEDWINDOW;
                exStyle = 0;
                auto windowRect = RECT{0, 0, static_cast<LONG>(w), static_cast<LONG>(h)};
                AdjustWindowRect(&windowRect, style, FALSE);
                x = config.x == -1 ?
                    (GetSystemMetrics(SM_CXSCREEN) - (windowRect.right - windowRect.left)) / 2 :
                    config.x;
                y = config.y == -1 ?
                (GetSystemMetrics(SM_CYSCREEN) - (windowRect.bottom - windowRect.top)) / 2 :
                    config.y;
            }
        }

        const auto hwnd = CreateWindowEx(
            exStyle,
            windowClass.lpszClassName,
            config.title.c_str(),
            style,
            x, y,
            w, h,
            nullptr,
            nullptr,
            hInstance,
            nullptr);
        if (hwnd == nullptr) { throw Exception("Error creating window", std::to_string(GetLastError())); }

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        return hwnd;
    }

    LRESULT CALLBACK Window::windowProcedure(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) {
        auto* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        switch (message) {
        case WM_SIZE:
            window->resize();
            return 0;
        case WM_CLOSE:
            window->close();
            break;
        default:;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    BOOL CALLBACK Window::monitorEnumProc(HMONITOR, HDC , const LPRECT lprcMonitor, const LPARAM dwData) {
        const auto data = reinterpret_cast<MonitorEnumData*>(dwData);
        if (data->enumIndex == data->monitorIndex) {
            data->monitorRect = *lprcMonitor;
            return FALSE;
        }
        data->enumIndex++;
        return TRUE;
    }

}