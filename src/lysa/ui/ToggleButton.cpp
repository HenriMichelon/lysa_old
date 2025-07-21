/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.toggle_button;

import lysa.ui.event;

namespace lysa::ui {

    ToggleButton::ToggleButton(): CheckWidget(TOGGLEBUTTON) {
        moveChildrenOnPush = true;
        redrawOnMouseEvent = true;
        allowFocus = true;
    }

    bool ToggleButton::eventMouseDown(const MouseButton button, const float x, const float y)  {
        const bool r = CheckWidget::eventMouseDown(button, x, y);
        if (getRect().contains(x, y)) {
            auto event = EventClick{};
            emit(Event::OnClick, &event);
            return event.consumed;
        }
        return r;
    }

}
