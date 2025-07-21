/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.text;

import lysa.math;
import lysa.ui.rect;
import lysa.ui.widget;

export namespace lysa::ui {
    /**
     * %A widget to display a line of text
     */
    class Text : public Widget {
    public:
        Text(const std::string& text);

        const auto& getText() const { return text; }

        void setText(const std::string& text);

        void setTextColor(const float4 &c);

        auto getTextColor() const { return textColor; }

        /**
         * Returns the size for the text.
         */
        void getSize(float &width, float &height);

        void _setSize(float width, float height) override;

    protected:
        Rect _getDefaultRect() override;

    private:
        std::string text;
        float4 textColor{0.0f};

        void eventCreate() override;
    };
}
