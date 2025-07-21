/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.frame;

import lysa.math;
import lysa.ui.panel;

export namespace lysa::ui {

    /**
     * %A rectangular frame with an optional title
     */
    class Frame: public Panel {
    public:
        // Create a Frame widget with an optional title
        Frame(const std::string& title = ""): Panel{FRAME}, title{title} {}

        // Return the current title of the widget
        auto& getTitle() const { return title; }

        // Change the title of the widget
        void setTitle(const std::string& T);

        void setTitleColor(const float4& c) { textColor = c; }

        auto getTitleColor() const { return textColor; }

    private:
        std::string title{};
        float4 textColor{1.0f};
    };

}
