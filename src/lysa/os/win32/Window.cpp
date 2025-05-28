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
import lysa.input;

namespace lysa {

    bool Window::resettingMousePosition{false};

    std::map<MouseCursor, HCURSOR> Window::mouseCursors;

    void Window::show() const {
        ShowWindow(static_cast<HWND>(windowHandle), SW_SHOW);
    }

    void* Window::createWindow() {
        if (mouseCursors.empty()) {
            mouseCursors[MouseCursor::ARROW]    = LoadCursor(nullptr, IDC_ARROW);
            mouseCursors[MouseCursor::WAIT]     = LoadCursor(nullptr, IDC_WAIT);
            mouseCursors[MouseCursor::RESIZE_H] = LoadCursor(nullptr, IDC_SIZEWE);
            mouseCursors[MouseCursor::RESIZE_V] = LoadCursor(nullptr, IDC_SIZENS);
        }

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

        rect.left = x;
        rect.top = y;
        rect.right = rect.left + w;
        rect.bottom = rect.top + h;
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
                if (!IsIconic(hWnd)) {
                    window->resize();
                }
                return 0;
            case WM_CLOSE:
                window->close();
                break;
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MOUSEWHEEL:
            case WM_MOUSEMOVE:
                return Input::windowProcedure(hWnd, message, wParam, lParam);
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

    float2 Window::getMousePosition() const {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(reinterpret_cast<HWND>(windowHandle), &point);
        return { point.x, point.y };
    }

    void Window::setMousePosition(const float2& position) const {
        POINT point{static_cast<int>(position.x), static_cast<int>(position.y)};
        ClientToScreen(reinterpret_cast<HWND>(windowHandle), &point);
        SetCursorPos(point.x, point.y);
    }

    void Window::setMouseCursor(const MouseCursor cursor) const {
        SetCursor(mouseCursors[cursor]);
        PostMessage(reinterpret_cast<HWND>(windowHandle), WM_SETCURSOR, 0, 0);
    }

    void Window::resetMousePosition() const {
        resettingMousePosition = true;
        SetCursorPos(rect.left + (rect.right-rect.left) / 2,
                         rect.top + (rect.bottom-rect.top) / 2);
    }

    void Window::setMouseMode(const MouseMode mode) const {
        MSG msg;
        while (PeekMessageW(&msg, reinterpret_cast<HWND>(windowHandle), 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        switch (mode) {
        case MouseMode::VISIBLE:
            ReleaseCapture();
            ClipCursor(nullptr);
            ShowCursor(TRUE);
            resetMousePosition();
            break;
        case MouseMode::HIDDEN:
            ReleaseCapture();
            ClipCursor(nullptr);
            ShowCursor(FALSE);
            break;
        case MouseMode::VISIBLE_CAPTURED: {
            SetCapture(reinterpret_cast<HWND>(windowHandle));
            ClipCursor(&rect);
            ShowCursor(TRUE);
            resetMousePosition();
            break;
        }
        case MouseMode::HIDDEN_CAPTURED: {
            SetCapture(reinterpret_cast<HWND>(windowHandle));
            ClipCursor(&rect);
            ShowCursor(FALSE);
            resetMousePosition();
            break;
        }
        default:
            throw Exception("Unknown mouse mode");
        }
    }

}