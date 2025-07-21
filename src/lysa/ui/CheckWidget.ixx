/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.check_widget;

import lysa.enums;
import lysa.ui.widget;

export namespace lysa::ui {
    /**
     * Super class for all two-states widgets
     */
    class CheckWidget : public Widget {
    public:
        //! State of the widget. Change on user action.
        enum State {
            //! Checked (aka ON)
            CHECK,
            //! Unchecked (aka OFF)
            UNCHECK
        };

        //! Return the current state of the widget
        auto getState() const { return state; }

        //! Change the state of the widget
        virtual void setState(State newState);

    protected:
        explicit CheckWidget(const Type type): Widget{type} { }

        bool eventMouseDown(MouseButton button, float x, float y) override;

    private:
        State state{UNCHECK};
    };
}
