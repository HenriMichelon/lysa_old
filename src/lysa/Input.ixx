/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>
#endif
export module lysa.input;

import lysa.enums;
import lysa.input_event;
import lysa.math;
import lysa.types;
import lysa.window;

export namespace lysa {

    /**
     * Describes a single input condition that can trigger an action.
     * An action may be composed of multiple entries (logical OR between them).
     */
    struct InputActionEntry {
        /** Source device type for this entry. */
        enum Type { KEYBOARD, MOUSE, GAMEPAD };
        /** Device/type associated with the value field. */
        Type  type;
        /** Encoded control identifier:
         *  - KEYBOARD: cast of Key enum to uint8
         *  - MOUSE: cast of MouseButton enum to uint8
         *  - GAMEPAD: cast of GamepadButton enum to uint8
         */
        uint8 value;
        /** Whether the action triggers on a press (true) or release (false). */
        bool  pressed{true};
    };

    /**
     * High-level action abstraction combining one or more input entries.
     * If any entry condition is satisfied, the action is considered active.
     */
    struct InputAction {
        /** Unique action name used to query and match events. */
        std::string name;
        /** List of device-specific entries that can trigger this action. */
        std::vector<InputActionEntry> entries;
    };

    /**
     * Static input helper responsible for keyboard, mouse and gamepad state.
     *
     * Responsibilities:
     *  - Polling and edge-detection for keys, mouse buttons and gamepad buttons
     *  - Normalized joystick vectors with deadzone handling
     *  - Action mapping utilities (InputAction)
     *
     * Notes:
     *  - All methods are static; there is no instance to create.
     *  - On Windows, the implementation integrates with Win32 messages,
     *    XInput and/or DirectInput to keep states updated.
     */
    class Input {
    public:
        /**
         * Returns true while the given key is currently held down.
         *
         * @param key Logical key code.
         * @return True if the key is down; false otherwise.
         */
        static bool isKeyPressed(Key key);

        /**
         * Returns true only on the frame the user starts pressing the key.
         * Subsequent frames while held return false until released and pressed again.
         *
         * @param key Logical key code.
         * @return True on the rising edge (down transition) for this key.
         */
        static bool isKeyJustPressed(Key key);

        /**
         * Returns true only on the frame the user releases the key.
         *
         * @param key Logical key code.
         * @return True on the falling edge (up transition) for this key.
         */
        static bool isKeyJustReleased(Key key);

        /**
         * Builds a 2D vector from four directional keys.
         * The result is not normalized; its components are in {-1, 0, 1}.
         *
         * @param negX Key for negative X (left).
         * @param posX Key for positive X (right).
         * @param negY Key for negative Y (down).
         * @param posY Key for positive Y (up).
         * @return A float2 with x and y components in [-1, 1].
         */
        static float2 getKeyboardVector(Key negX, Key posX, Key negY, Key posY);

        /**
         * Returns true while the given mouse button is currently held down.
         *
         * @param mouseButton Mouse button identifier.
         * @return True if the button is down; false otherwise.
         */
        static bool isMouseButtonPressed(MouseButton mouseButton);

        /**
         * Returns true only on the frame the user starts pressing the mouse button.
         *
         * @param mouseButton Mouse button identifier.
         * @return True on the rising edge (down transition) for this button.
         */
        static bool isMouseButtonJustPressed(MouseButton mouseButton);
        /**
         * Returns true only on the frame the user releases the mouse button.
         *
         * @param mouseButton Mouse button identifier.
         * @return True on the falling edge (up transition) for this button.
         */
        static bool isMouseButtonJustReleased(MouseButton mouseButton);

        /**
         * Returns the number of connected joypads (gamepads included).
         *
         * @return Count of currently connected devices.
         */
        static uint32 getConnectedJoypads();

        /**
         * Returns true if the device at index is recognized as a gamepad.
         *
         * @param index Index in range [0 .. getConnectedJoypads()).
         * @return True if the device is a gamepad; false otherwise.
         */
        static bool isGamepad(uint32 index);

        /**
         * Returns a human-readable name for the joypad at index.
         *
         * @param index Index in range [0 .. getConnectedJoypads()).
         * @return UTF-8 device name or an empty string if unavailable.
         */
        static std::string getJoypadName(uint32 index);

        /**
         * Returns the 2D vector for a gamepad joystick axis pair.
         * Deadzone is applied internally and output is normalized to [-1, 1].
         *
         * @param index Index in range [0 .. getConnectedJoypads()).
         * @param axisJoystick Joystick selector (e.g., left or right stick).
         * @return A float2 with x/y in [-1, 1] after deadzone.
         */
        static float2 getGamepadVector(uint32 index, GamepadAxisJoystick axisJoystick);

        /**
         * Returns true while the given gamepad button is currently held down.
         *
         * @param index Index in range [0 .. getConnectedJoypads()).
         * @param gamepadButton Button identifier.
         * @return True if the button is down; false otherwise.
         */
        static bool isGamepadButtonPressed(uint32 index, GamepadButton gamepadButton);
        //static float getGamepadAxisValue(uint32_t index, GamepadAxis gamepadAxis);

        /** Returns true on the frame the specified gamepad button is released. */
        static bool isGamepadButtonJustReleased(GamepadButton button);
        /** Returns true on the frame the specified gamepad button is pressed. */
        static bool isGamepadButtonJustPressed(GamepadButton button);

        /**
         * Register a high-level input action that can be matched against events.
         * If an action with the same name exists, it is replaced.
         */
        static void addAction(const InputAction& action);
        /**
         * Checks whether the given input event matches the named action.
         *
         * @param actionName Name of a previously registered action.
         * @param inputEvent Concrete input event to test.
         * @return True if inputEvent satisfies any entry of the action.
         */
        static bool isAction(const std::string& actionName, const InputEvent &inputEvent);

    private:
        // Registered actions indexed by name.
        static inline std::map<std::string, InputAction> inputActions;

        // Current pressed state for each key.
        static std::unordered_map<Key, bool> keyPressedStates;
        // Rising-edge state for each key (true only on press frame).
        static std::unordered_map<Key, bool> keyJustPressedStates;
        // Falling-edge state for each key (true only on release frame).
        static std::unordered_map<Key, bool> keyJustReleasedStates;
        // Current pressed state for each mouse button.
        static std::unordered_map<MouseButton, bool> mouseButtonPressedStates;
        // Rising-edge state for each mouse button.
        static std::unordered_map<MouseButton, bool> mouseButtonJustPressedStates;
        // Falling-edge state for each mouse button.
        static std::unordered_map<MouseButton, bool> mouseButtonJustReleasedStates;
        // Current pressed state for each gamepad button (aggregated).
        static std::unordered_map<GamepadButton, bool> gamepadButtonPressedStates;
        // Rising-edge state for each gamepad button.
        static std::unordered_map<GamepadButton, bool> gamepadButtonJustPressedStates;
        // Falling-edge state for each gamepad button.
        static std::unordered_map<GamepadButton, bool> gamepadButtonJustReleasedStates;

        /* Initializes platform input backends and state tables. */
        static void initInput();

        /* Shuts down platform input backends and clears states. */
        static void closeInput();

        /* Converts an engine Key to the platform-specific OsKey. */
        static OsKey keyToOsKey(Key key);

        /* Converts a platform-specific OsKey to the engine Key. */
        static Key osKeyToKey(OsKey key);

        /* Applies a symmetrical deadzone percentage to a raw axis value in [-1, 1]. */
        static float applyDeadzone(float value, float deadzonePercent);

        /* Emits a synthetic gamepad button event toward the window/event system. */
        static void generateGamepadButtonEvent(Window&, GamepadButton,bool);

        friend class Window;
        friend class Application;

#ifdef _WIN32
        // DirectInput axis range (symmetric); used for normalization.
        static const int DI_AXIS_RANGE;
        // Precomputed 1.0f / DI_AXIS_RANGE for fast normalization.
        static const float DI_AXIS_RANGE_DIV;
        // True if XInput is available/selected for gamepad queries.
        static bool useXInput;
        /* Updates all input states for the given window (Win32 message pump tick). */
        static void updateInputStates(Window&);
        /* Win32 window procedure used to intercept input messages. */
        static LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        /* DirectInput enumeration callback for device objects (axes, buttons). */
        static BOOL CALLBACK deviceObjectCallback(const DIDEVICEOBJECTINSTANCEW *doi, void *user);
        /* DirectInput enumeration callback for gamepad devices. */
        static BOOL CALLBACK enumGamepadsCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);
#endif
    };

}
