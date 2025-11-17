/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.toggle_button;

import lysa.enums;
import lysa.ui.check_widget;

export namespace lysa::ui {
    /**
     * Two states clickable button
     */
    class ToggleButton : public CheckWidget {
    public:
        ToggleButton();

    protected:
        bool eventMouseUp(MouseButton button, float x, float y) override;
    };
}
