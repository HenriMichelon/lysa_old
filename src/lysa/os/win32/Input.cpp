/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#define NOMINMAX
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <windowsx.h>
#include <Xinput.h>
#include <dinput.h>
#include "mappings.h"
module lysa.input;

namespace lysa {

    bool        Input::useXInput{false};
    const int   Input::DI_AXIS_RANGE{1000};
    const float Input::DI_AXIS_RANGE_DIV{1000.5f};

    struct _DirectInputState {
        LPDIRECTINPUTDEVICE8 device;
        std::string name;
        std::array<float, 6> axes; // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
        std::array<bool, 32> buttons;
        // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
        int                                 indexAxisLeftX{0};
        int                                 indexAxisLeftY{0};
        int                                 indexAxisRightX{0};
        int                                 indexAxisRightY{0};
        std::array<int, static_cast<int>(GamepadButton::LAST) + 1> indexButtons;
    };

    static std::map<uint32, _DirectInputState> _directInputStates{};
    static std::map<uint32, XINPUT_STATE>      _xinputStates{};
    static LPDIRECTINPUT8                      _directInput = nullptr;

    static std::map<GamepadButton, int> GAMEPABUTTON2XINPUT{
            {GamepadButton::A, XINPUT_GAMEPAD_A},
            {GamepadButton::B, XINPUT_GAMEPAD_B},
            {GamepadButton::X, XINPUT_GAMEPAD_X},
            {GamepadButton::Y, XINPUT_GAMEPAD_Y},
            {GamepadButton::LB, XINPUT_GAMEPAD_LEFT_SHOULDER},
            {GamepadButton::RB, XINPUT_GAMEPAD_RIGHT_SHOULDER},
            {GamepadButton::LT, XINPUT_GAMEPAD_LEFT_THUMB},
            {GamepadButton::RT, XINPUT_GAMEPAD_RIGHT_THUMB},
            {GamepadButton::BACK, XINPUT_GAMEPAD_BACK},
            {GamepadButton::START, XINPUT_GAMEPAD_START},
            {GamepadButton::DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN},
            {GamepadButton::DPAD_UP, XINPUT_GAMEPAD_DPAD_UP},
            {GamepadButton::DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT},
            {GamepadButton::DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT},
            {GamepadButton::DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT},
    };

    BOOL CALLBACK Input::deviceObjectCallback(const DIDEVICEOBJECTINSTANCEW *doi, void *user) {
        const auto *data = reinterpret_cast<_DirectInputState *>(user);
        if (DIDFT_GETTYPE(doi->dwType) & DIDFT_AXIS) {
            DIPROPRANGE dipr;
            ZeroMemory(&dipr, sizeof(dipr));
            dipr.diph.dwSize       = sizeof(dipr);
            dipr.diph.dwHeaderSize = sizeof(dipr.diph);
            dipr.diph.dwObj        = doi->dwType;
            dipr.diph.dwHow        = DIPH_BYID;
            dipr.lMin              = -DI_AXIS_RANGE;
            dipr.lMax              = DI_AXIS_RANGE;
            if (FAILED(data->device->SetProperty(DIPROP_RANGE, &dipr.diph))) {
                return DIENUM_CONTINUE;
            }
        }
        return DIENUM_CONTINUE;
    }

    BOOL CALLBACK Input::enumGamepadsCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *) {
        if (_directInput) {
            _DirectInputState state{
                .device = nullptr,
                .name = std::string{to_string(pdidInstance->tszProductName)}
            };
            if (FAILED(_directInput->CreateDevice(pdidInstance->guidInstance,
                &state.device,
                nullptr))) {
                return DIENUM_CONTINUE;
            }
            if (FAILED(state.device->SetDataFormat(&c_dfDIJoystick))) {
                return DIENUM_CONTINUE;
            }
            if (FAILED(state.device->SetCooperativeLevel(nullptr, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
                return DIENUM_CONTINUE;
            }
            if (FAILED(state.device->Acquire())) {
                return DIENUM_CONTINUE;
            }

            if (FAILED(state.device->EnumObjects(deviceObjectCallback,
                &state,
                DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV))) {
                state.device->Release();
                return DIENUM_CONTINUE;
            }

            // Generate a joystick GUID that matches the SDL 2.0.5+ one
            // https://github.com/glfw/glfw/blob/master/src/win32_joystick.c#L452
            char guid[33];
            char name[256];
            if (!WideCharToMultiByte(CP_UTF8,
                                     0,
                                     pdidInstance->tszInstanceName,
                                     -1,
                                     name,
                                     sizeof(name),
                                     nullptr,
                                     nullptr)) {
                state.device->Release();
                return DIENUM_CONTINUE;
            }
            if (memcmp(&pdidInstance->guidProduct.Data4[2], "PIDVID", 6) == 0) {
                std::sprintf(guid,
                        "03000000%02x%02x0000%02x%02x000000000000",
                        static_cast<uint8>(pdidInstance->guidProduct.Data1),
                        static_cast<uint8>(pdidInstance->guidProduct.Data1 >> 8),
                        static_cast<uint8>(pdidInstance->guidProduct.Data1 >> 16),
                        static_cast<uint8>(pdidInstance->guidProduct.Data1 >> 24));
            } else {
                std::sprintf(guid,
                        "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                        name[0],
                        name[1],
                        name[2],
                        name[3],
                        name[4],
                        name[5],
                        name[6],
                        name[7],
                        name[8],
                        name[9],
                        name[10]);
            }

            auto sguid = std::string{guid};
            for (auto &_inputDefaultMapping : _inputDefaultMappings) {
                auto mapping = split(_inputDefaultMapping, ',');
                if (mapping[0] == sguid) {
                    for (int i = 2; i < mapping.size(); i++) {
                        auto parts = split(mapping[i], ':');
                        if ((parts.size() > 1) && (parts[1].size() > 1)) {
                            try {
                                const auto index = stoi(std::string{parts[1].substr(1)});
                                if (parts[0] == "leftx")
                                    state.indexAxisLeftX = index;
                                if (parts[0] == "lefty")
                                    state.indexAxisLeftY = index;
                                if (parts[0] == "rightx")
                                    state.indexAxisRightX = index;
                                if (parts[0] == "righty")
                                    state.indexAxisRightY = index;
                                if (parts[0] == "a")
                                    state.indexButtons[static_cast<int>(GamepadButton::A)] = index;
                                if (parts[0] == "b")
                                    state.indexButtons[static_cast<int>(GamepadButton::B)] = index;
                                if (parts[0] == "x")
                                    state.indexButtons[static_cast<int>(GamepadButton::X)] = index;
                                if (parts[0] == "y")
                                    state.indexButtons[static_cast<int>(GamepadButton::Y)] = index;
                                if (parts[0] == "leftshoulder")
                                    state.indexButtons[static_cast<int>(GamepadButton::LB)] = index;
                                if (parts[0] == "rightshoulder")
                                    state.indexButtons[static_cast<int>(GamepadButton::RB)] = index;
                                if (parts[0] == "leftstick")
                                    state.indexButtons[static_cast<int>(GamepadButton::LT)] = index;
                                if (parts[0] == "rightstick")
                                    state.indexButtons[static_cast<int>(GamepadButton::RT)] = index;
                                if (parts[0] == "back")
                                    state.indexButtons[static_cast<int>(GamepadButton::BACK)] = index;
                                if (parts[0] == "start")
                                    state.indexButtons[static_cast<int>(GamepadButton::START)] = index;
                            } catch (const std::invalid_argument &) {
                            }
                        }
                    }
                    _directInputStates[_directInputStates.size()] = state;
                }
            }
        }
        return DIENUM_CONTINUE;
    }

    void Input::initInput() {
        for (uint32 i = 0; i < XUSER_MAX_COUNT; ++i) {
            XINPUT_STATE state;
            ZeroMemory(&state, sizeof(XINPUT_STATE));
            if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                _xinputStates[i] = state;
            }
        }
        useXInput = !_xinputStates.empty();
        if (!useXInput) {
            if (FAILED(DirectInput8Create(GetModuleHandle(nullptr),
                DIRECTINPUT_VERSION,
                IID_IDirectInput8,
                reinterpret_cast<void**>(&_directInput), nullptr))) {
                throw Exception("DirectInput8Create failed");
            }
            _directInput->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                      enumGamepadsCallback,
                                      nullptr,
                                      DIEDFL_ATTACHEDONLY);
        }
    }

    void Input::closeInput() {
        if (_directInput) {
            for (const auto entry : _directInputStates) {
                entry.second.device->Release();
            }
            _directInputStates.clear();
            _directInput->Release();
            _directInput = nullptr;
        }
    }

    uint32 Input::getConnectedJoypads() {
        uint32 count = 0;
        if (useXInput) {
            count = _xinputStates.size();
        } else {
            count = _directInputStates.size();
        }
        return count;
    }

    bool Input::isGamepad(const uint32 index) {
        if (useXInput) {
            if (_xinputStates.contains(index)) {
                XINPUT_CAPABILITIES xinputCapabilities;
                ZeroMemory(&xinputCapabilities, sizeof(XINPUT_CAPABILITIES));
                if (XInputGetCapabilities(index, 0, &xinputCapabilities) == ERROR_SUCCESS) {
                    return xinputCapabilities.SubType == XINPUT_DEVSUBTYPE_GAMEPAD;
                }
            }
            return false;
        } else {
            return _directInputStates.contains(index);
        }
    }

    void Input::generateGamepadButtonEvent(Window& window, const GamepadButton button, const bool pressed) {
        if (pressed && (!gamepadButtonPressedStates[button])) {
            gamepadButtonJustPressedStates[button] = true;
            gamepadButtonJustReleasedStates[button] = false;
            auto event = InputEventGamepadButton(button, pressed);
            window.input(event);
        }
        if ((!pressed) && (gamepadButtonPressedStates[button])) {
            gamepadButtonJustPressedStates[button] = false;
            gamepadButtonJustReleasedStates[button] = true;
            auto event = InputEventGamepadButton(button, pressed);
            window.input(event);
        }
        gamepadButtonPressedStates[button] = pressed;
    }

    void Input::updateInputStates(Window& window) {
        if (useXInput) {
            _xinputStates.clear();
            for (uint32_t i = 0; i < XUSER_MAX_COUNT; ++i) {
                XINPUT_STATE state;
                ZeroMemory(&state, sizeof(XINPUT_STATE));
                if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                    _xinputStates[i] = state;
                    for (int i = 0; i < static_cast<int>(GamepadButton::LAST); i++) {
                        auto button = static_cast<GamepadButton>(i);
                        generateGamepadButtonEvent(window, button, state.Gamepad.wButtons & GAMEPABUTTON2XINPUT[button]);
                    }
                }
            }
        } else {
            for (auto &entry : _directInputStates) {
                auto &  gamepad = entry.second;
                HRESULT hr      = gamepad.device->Poll();
                if (FAILED(hr)) {
                    hr = gamepad.device->Acquire();
                    while (hr == DIERR_INPUTLOST) {
                        hr = gamepad.device->Acquire();
                    }
                    if (FAILED(hr)) {
                        _directInputStates.erase(entry.first);
                        continue;
                    }
                }

                DIJOYSTATE state{0};
                if (FAILED(gamepad.device->GetDeviceState(sizeof(state), &state))) {
                    continue;
                }
                gamepad.axes[0] = (static_cast<float>(state.lX) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[1] = (static_cast<float>(state.lY) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[2] = (static_cast<float>(state.lZ) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[3] = (static_cast<float>(state.lRx) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[4] = (static_cast<float>(state.lRy) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[5] = (static_cast<float>(state.lRz) + 0.5f) / DI_AXIS_RANGE_DIV;
                for (int i = 0; i < 32; i++) {
                    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
                    gamepad.buttons[i] = (state.rgbButtons[i] & 0x80);
                }
                for (int i = 0; i < static_cast<int>(GamepadButton::LAST); i++) {
                    auto button = static_cast<GamepadButton>(i);
                    generateGamepadButtonEvent(window, button, gamepad.buttons[gamepad.indexButtons[static_cast<int>(button)]]);
                }
            }
        }
    }

    bool Input::isGamepadButtonPressed(const uint32 index, const GamepadButton gamepadButton) {
        if (useXInput && _xinputStates.contains(index)) {
            return _xinputStates[index].Gamepad.wButtons & GAMEPABUTTON2XINPUT[gamepadButton];
        } else if (_directInputStates.contains(index)) {
            const auto &gamepad = _directInputStates[index];
            return gamepad.buttons[gamepad.indexButtons[static_cast<int>(gamepadButton)]];
        }
        return false;
    }

    float2 Input::getGamepadVector(const uint32 index, const GamepadAxisJoystick axisJoystick) {
        if (useXInput && _xinputStates.contains(index)) {
            const auto gamepad        = _xinputStates[index].Gamepad;
            const auto xAxis          = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.sThumbLX : gamepad.sThumbRX;
            const auto yAxis          = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.sThumbLY : gamepad.sThumbRY;
            const auto deadzonPercent = ((axisJoystick == GamepadAxisJoystick::LEFT
                    ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
                    : XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                / 32767.0f);
            const float2 vector{
                    applyDeadzone(xAxis / 32767.0f, deadzonPercent),
                    applyDeadzone(-yAxis / 32767.0f, deadzonPercent)
            };
            const float l = length(vector);
            return (l > 1.0f) ? vector / l : vector;
        } else if (_directInputStates.contains(index)) {
            const auto &gamepad = _directInputStates[index];
            const auto  xAxis   = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.indexAxisLeftX : gamepad.indexAxisRightX;
            const auto  yAxis   = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.indexAxisLeftY : gamepad.indexAxisRightY;
            const float2  vector{
                    applyDeadzone(gamepad.axes[xAxis], 0.05f),
                    applyDeadzone(gamepad.axes[yAxis], 0.05f)
            };
            const float l = length(vector);
            return (l > 1.0f) ? vector / l : vector;
        }
        return FLOAT2ZERO;
    }

    std::string Input::getJoypadName(const uint32 index) {
        if (useXInput) {
            return "XInput";
        }
        if (_directInputStates.contains(index)) {
            return _directInputStates[index].name;
        }
        return "??";
    }

    float2 Input::getKeyboardVector(const Key keyNegX, const Key keyPosX, const Key keyNegY, const Key keyPosY) {
        const auto  x = keyPressedStates[keyNegX] ? -1 : keyPressedStates[keyPosX] ? 1 : 0;
        const auto  y = keyPressedStates[keyNegY] ? -1 : keyPressedStates[keyPosY] ? 1 : 0;
        const float2  vector{x, y};
        const float l = length(vector);
        return (l > 1.0f) ? vector / l : vector;
    }


    // Helper to translate Windows get state to lysa key modifiers
    int _getKeyboardModifiers() {
        int modifiers = 0;
        if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= static_cast<int>(KeyModifier::SHIFT);
        if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= static_cast<int>(KeyModifier::CONTROL);
        if (GetKeyState(VK_MENU) & 0x8000) modifiers |= static_cast<int>(KeyModifier::ALT);
        return modifiers;
    }

    uint32 _getMouseButtonState(const WPARAM wParam) {
        uint32 state{0};
        if (wParam & MK_LBUTTON) state += static_cast<int>(MouseButton::LEFT);
        if (wParam & MK_MBUTTON) state += static_cast<int>(MouseButton::MIDDLE);
        if (wParam & MK_RBUTTON) state += static_cast<int>(MouseButton::RIGHT);
        return state;
    }

    LRESULT CALLBACK Input::windowProcedure(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) {
        static float lastMouseX = -1.0f;
        static float lastMouseY = -1.0f;
        auto* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        switch (message) {
            case WM_KEYDOWN:{
                const auto scanCode = static_cast<OsKey>((lParam & 0x00FF0000) >> 16);
                const auto key = osKeyToKey(scanCode);
                keyJustPressedStates[key] = !keyPressedStates[key];
                keyPressedStates[key] = true;
                keyJustReleasedStates[key] = false;
                if (keyJustPressedStates[key]) {
                    auto event = InputEventKey{key, true, static_cast<int>(lParam & 0xFFFF), _getKeyboardModifiers()};
                    window->input(event);
                }
                break;
            }
            case WM_KEYUP: {
                auto scanCode = static_cast<OsKey>((lParam & 0x00FF0000) >> 16);
                auto key = osKeyToKey(scanCode);
                keyPressedStates[key] = false;
                keyJustPressedStates[key] = false;
                keyJustReleasedStates[key] = true;
                auto event = InputEventKey{key, false, static_cast<int>(lParam & 0xFFFF), _getKeyboardModifiers()};
                window->input(event);
                break;
            }
            case WM_LBUTTONDOWN: {
                mouseButtonJustPressedStates[MouseButton::LEFT] = !mouseButtonPressedStates[MouseButton::LEFT];
                mouseButtonPressedStates[MouseButton::LEFT] = true;
                mouseButtonJustReleasedStates[MouseButton::LEFT] = false;
                auto event = InputEventMouseButton(MouseButton::LEFT,
                                                       true,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getExtent().height)-GET_Y_LPARAM(lParam));
                window->input(event);
                break;
            }
            case WM_LBUTTONUP: {
                mouseButtonJustPressedStates[MouseButton::LEFT] = false;
                mouseButtonPressedStates[MouseButton::LEFT] = false;
                mouseButtonJustReleasedStates[MouseButton::LEFT] = false;
                auto event = InputEventMouseButton(MouseButton::LEFT,
                                                       false,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getExtent().height)-GET_Y_LPARAM(lParam));
                window->input(event);
                break;
            }
            case WM_RBUTTONDOWN: {
                mouseButtonJustPressedStates[MouseButton::RIGHT] = !mouseButtonPressedStates[MouseButton::RIGHT];
                mouseButtonPressedStates[MouseButton::RIGHT] = true;
                mouseButtonJustReleasedStates[MouseButton::RIGHT] = false;
                auto event = InputEventMouseButton(MouseButton::RIGHT,
                                                       true,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getExtent().height)-GET_Y_LPARAM(lParam));
                window->input(event);
                break;
            }
            case WM_RBUTTONUP: {
                mouseButtonJustPressedStates[MouseButton::RIGHT] = false;
                mouseButtonPressedStates[MouseButton::RIGHT] = false;
                mouseButtonJustReleasedStates[MouseButton::RIGHT] = false;
                auto event = InputEventMouseButton(MouseButton::RIGHT,
                                                       false,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getExtent().height)-GET_Y_LPARAM(lParam));
                window->input(event);
                break;
            }
            case WM_MBUTTONDOWN: {
                mouseButtonJustPressedStates[MouseButton::MIDDLE] = !mouseButtonPressedStates[MouseButton::MIDDLE];
                mouseButtonPressedStates[MouseButton::MIDDLE] = true;
                mouseButtonJustReleasedStates[MouseButton::MIDDLE] = false;
                auto event = InputEventMouseButton(MouseButton::MIDDLE,
                                                       true,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getExtent().height)-GET_Y_LPARAM(lParam));
                window->input(event);
                break;
            }
            case WM_MBUTTONUP: {
                mouseButtonJustPressedStates[MouseButton::MIDDLE] = false;
                mouseButtonPressedStates[MouseButton::MIDDLE] = false;
                mouseButtonJustReleasedStates[MouseButton::MIDDLE] = false;
                auto event = InputEventMouseButton(MouseButton::MIDDLE,
                                                       false,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getExtent().height)-GET_Y_LPARAM(lParam));
                window->input(event);
                break;
            }
            case WM_MOUSEWHEEL: {
                mouseButtonJustPressedStates[MouseButton::MIDDLE] = false;
                mouseButtonPressedStates[MouseButton::MIDDLE] = false;
                mouseButtonJustReleasedStates[MouseButton::MIDDLE] = false;
                auto event = InputEventMouseButton(MouseButton::WHEEL,
                                                       GET_WHEEL_DELTA_WPARAM(wParam) < 0,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(GET_KEYSTATE_WPARAM(wParam)),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getExtent().height)-GET_Y_LPARAM(lParam));
                window->input(event);
                break;
            }
            case WM_MOUSEMOVE: {
                auto xPos = static_cast<float>(GET_X_LPARAM(lParam));
                auto yPos = 0.0f;
                auto y = GET_Y_LPARAM(lParam);
                const auto height = window->getExtent().height;
                if (y < height) {
                    yPos = static_cast<float>(height-y);
                } else if (y < 0){
                    yPos = static_cast<float>(height);
                }
                if (!Window::resettingMousePosition) {
                    if ((lastMouseX != -1) && (lastMouseY != -1)) {
                        auto dx = xPos - lastMouseX;
                        auto dy = yPos - lastMouseY;
                        auto event = InputEventMouseMotion(_getMouseButtonState(wParam), _getKeyboardModifiers(), xPos, yPos, dx, dy);
                        window->input(event);
                    } else {
                        auto event = InputEventMouseMotion(_getMouseButtonState(wParam), _getKeyboardModifiers(), xPos, yPos, 0, 0);
                        window->input(event);
                    }
                } else {
                    Window::resettingMousePosition = false;
                }
                lastMouseX = xPos;
                lastMouseY = yPos;
                break;
            }
            default:;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

}
