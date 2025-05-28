/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#include <windows.h>
#endif
export module lysa.input;

import lysa.global;
import lysa.input_event;
import lysa.window;

export namespace lysa {

    struct InputActionEntry {
        enum Type { KEYBOARD, MOUSE, GAMEPAD };
        Type  type;
        uint8 value;
        bool  pressed{true};
    };

    struct InputAction {
        std::string name;
        std::vector<InputActionEntry> entries;
    };

    /**
     * %A singleton for handling inputs
     */
    class Input {
    public:
        /**
         * Returns true if you are pressing the key
         */
        static bool isKeyPressed(Key key);

        /**
         * Returns true when the user has started pressing the key
         */
        static bool isKeyJustPressed(Key key);

        /**
         * Returns true when the user stops pressing the key
         */
        static bool isKeyJustReleased(Key key);

        /**
         * Gets an input vector by specifying four keys for the positive and negative X and Y axes.
         */
        static float2 getKeyboardVector(Key negX, Key posX, Key negY, Key posY);

        /**
         * Returns true if you are pressing the mouse button
         */
        static bool isMouseButtonPressed(MouseButton mouseButton);

        /**
         * Returns true when the user has started pressing the mouse button
         */
        static bool isMouseButtonJustPressed(MouseButton mouseButton);
        /**
         * Returns true when the user stops pressing the mouse button
         */
        static bool isMouseButtonJustReleased(MouseButton mouseButton);

        /**
         * Returns the number of connected joypads, including gamepads
         */
        static uint32 getConnectedJoypads();

        /**
         * Returns true if the joypad is a gamepad
         * @param index index of the joypad in [0..getConnectedJoypads()]
         */
        static bool isGamepad(uint32 index);

        /**
         * Returns the joypad name
         * @param index index of the joypad in [0..getConnectedJoypads()]
         */
        static std::string getJoypadName(uint32 index);

        /**
         * Gets an input vector for a gamepad joystick
         * @param index index of the joypad in [0..getConnectedJoypads()]
         * @param axisJoystick axis
         */
        static float2 getGamepadVector(uint32 index, GamepadAxisJoystick axisJoystick);

        /**
        * Returns true if you are pressing the gamepad button
        * @param index index of the joypad in [0..getConnectedJoypads()]
        * @param gamepadButton gamepad button
        */
        static bool isGamepadButtonPressed(uint32 index, GamepadButton gamepadButton);
        //static float getGamepadAxisValue(uint32_t index, GamepadAxis gamepadAxis);

        static bool isGamepadButtonJustReleased(GamepadButton button);
        static bool isGamepadButtonJustPressed(GamepadButton button);

        static void addAction(const InputAction& action);
        static bool isAction(const std::string& actionName, const InputEvent &inputEvent);

    private:
        static inline std::map<std::string, InputAction> inputActions;

        static float applyDeadzone(float value, float deadzonePercent);
        static void generateGamepadButtonEvent(Window&, GamepadButton, bool);

    public:
        static std::unordered_map<Key, bool> _keyPressedStates;
        static std::unordered_map<Key, bool> _keyJustPressedStates;
        static std::unordered_map<Key, bool> _keyJustReleasedStates;
        static std::unordered_map<MouseButton, bool> _mouseButtonPressedStates;
        static std::unordered_map<MouseButton, bool> _mouseButtonJustPressedStates;
        static std::unordered_map<MouseButton, bool> _mouseButtonJustReleasedStates;
        static std::unordered_map<GamepadButton, bool> _gamepadButtonPressedStates;
        static std::unordered_map<GamepadButton, bool> _gamepadButtonJustPressedStates;
        static std::unordered_map<GamepadButton, bool> _gamepadButtonJustReleasedStates;

        static OsKey keyToOsKey(Key key);
        static Key osKeyToKey(OsKey key);

#ifdef _WIN32
        static const int DI_AXIS_RANGE;
        static const float DI_AXIS_RANGE_DIV;
        static bool _useXInput;
        static void _initInput();
        static void _closeInput();
        static void _updateInputStates(Window&);
        static LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif
    };

}
