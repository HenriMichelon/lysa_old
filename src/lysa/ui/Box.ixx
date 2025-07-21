/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.box;

import lysa.ui.panel;

export namespace lysa::ui {

    /**
     * %A rectangular box
     */
    class Box: public Panel {
    public:
        Box(): Panel{BOX} {}

    protected:
        Box(const Type type): Panel{type} {}
    };

}