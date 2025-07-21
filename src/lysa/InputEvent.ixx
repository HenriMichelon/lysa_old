/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.input_event;

import lysa.enums;
import lysa.math;
import lysa.object;
import lysa.types;

export namespace lysa {

    /**
     * Base class of all input events
     */
    class InputEvent: public Object {
    public:
        /**
         * Returns the type of the event
         */
        InputEventType getType() const { return type; }

    protected:
        explicit InputEvent(const InputEventType type): type{type} {}

    private:
        InputEventType type;
    };

    /**
     * Keyboard input event
     */
    class InputEventKey: public InputEvent {
    public:
         InputEventKey(const Key key, const bool pressed, const int repeat, const int modifiers):
             InputEvent{InputEventType::KEY},
             keycode{key},
             repeat{repeat},
             pressed{pressed},
             modifiers{modifiers} {}

        /**
         * Returns the key code
         */
        Key getKey() const { return keycode; }

        /**
         * The repeat count for the current event. The value is the number of times the keystroke is auto-repeated as a result of the user holding down the key
         */
        auto getRepeatCount() const { return repeat; }

        /**
         * Returns true if the key is pressed
         */
        auto isPressed() const { return pressed; }

        /**
         * Returns the state of the z0::KeyModifier keys
         */
        auto getModifiers() const { return modifiers; }

    private:
        Key keycode;
        int repeat;
        bool pressed;
        int modifiers;
    };

    /**
     * Gamepad buttons event
     */
    class InputEventGamepadButton: public InputEvent {
    public:
        InputEventGamepadButton(const GamepadButton button, const bool pressed):
            InputEvent{InputEventType::GAMEPAD_BUTTON},
            button{button},
            pressed{pressed} {}

        /**
         * Return the gamepad button
         */
        auto getGamepadButton() const { return button; }

         /**
         * Returns true if the gamepad button is pressed
         */
        auto isPressed() const { return pressed; }

    private:
        GamepadButton button;
        bool pressed;
    };

    /**
     * Base mouse event
    */
    class InputEventMouse: public InputEvent {
    public:
        /**
         * Returns the current mouse position
         */
        auto getPosition() const { return float2{x, y}; }

        /**
         * Returns the current mouse x position
         */
        auto getX() const { return x; }

        /**
         * Returns the current mouse y position
         */
        auto getY() const { return y; }

        /**
         * Returns the mouse button states (which button is down)
         */
        auto getButtonsState() { return buttonsState; }

        /**
         * Returns the state of the z0::KeyModifier keys
        */
        auto getModifiers() const { return modifiers; }

    protected:
        InputEventMouse(const InputEventType type, const uint32 buttonsState, const int modifiers, const float posX, const float posY):
            InputEvent{type},
            x{posX},
            y{posY},
            buttonsState{buttonsState},
            modifiers{modifiers} {}

    private:
        float x, y;
        uint32 buttonsState;
        int modifiers;
    };


    /**
     * Mouse move event
    */
    class InputEventMouseMotion: public InputEventMouse {
    public:
        InputEventMouseMotion(const uint32 buttonsState, const int modifiers, const float posX, const float posY, const float rX, const float rY):
            InputEventMouse{InputEventType::MOUSE_MOTION, buttonsState, modifiers, posX, posY},
            relativeX{rX},
            relativeY{rY} {}

        /**
         * Returns the relative x movement
         */
        float getRelativeX() const { return relativeX; }

        /**
         * Returns the relative y movement
         */
        float getRelativeY() const { return relativeY; }

    private:
        float relativeX, relativeY;
    };

    /**
     * Mouse button pressed/released event
    */
    class InputEventMouseButton: public InputEventMouse {
    public:
        InputEventMouseButton(const MouseButton button, const bool pressed,const  int modifiers, const uint32 buttonsState, const float posX, const float posY):
            InputEventMouse{InputEventType::MOUSE_BUTTON, buttonsState, modifiers, posX, posY},
            button{button},
            pressed{pressed} {}

        /**
         * Returns the mouse button
         */
        MouseButton getMouseButton() const { return button; }

        /**
         * Returns true is the button is pressed
         */
        bool isPressed() const { return pressed; }

    private:
        MouseButton button;
        bool pressed;
    };
}
