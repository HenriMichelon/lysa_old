/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.check_widget;

import lysa.ui.event;

namespace lysa::ui {

    void CheckWidget::setState(const State newState) {
        if (state == newState) { return; }
        state = newState;
        resizeChildren();
        refresh();
        auto stat = EventState{.state = newState};
        emit(Event::OnStateChange, &stat);
    }

    bool CheckWidget::eventMouseDown(const MouseButton button, const float x, const float y) {
        if (getRect().contains(x, y)) {
            setState(state == CHECK ? UNCHECK : CHECK);
        }
        return Widget::eventMouseDown(button, x, y);
    }

}
