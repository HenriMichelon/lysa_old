/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.button;

import lysa.ui.event;

namespace lysa::ui {

    Button::Button(): Box{BUTTON} {
        moveChildrenOnPush = true;
        redrawOnMouseEvent = true;
        allowFocus = true;
    }

    bool Button::eventMouseUp(const MouseButton button, const float x, const float y) {
        const bool p = isPushed();
        if (p && (!getRect().contains(x, y))) {
            setPushed(false);
            resizeChildren();
            return Box::eventMouseUp(button, x, y);
        }
        const bool consumed = Box::eventMouseUp(button, x, y);
        if ((!consumed) && p) {
            auto event = EventClick{};
            emit(Event::OnClick, &event);
            return event.consumed;
        }
        return consumed;
    }
}
