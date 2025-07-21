/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.line;

import lysa.ui.widget;

export namespace lysa::ui {
    /**
     * %A horizontal or vertical line
     */
    class Line : public Widget {
    public:
        enum LineStyle { HORIZ, VERT };

        Line(LineStyle style = HORIZ);

        auto getStyle() const { return style; }

        void setStyle(LineStyle style);

    private:
        LineStyle style;
    };

    class HLine : public Line {
    public:
        HLine() : Line(HORIZ) {}
    };

    class VLine : public Line {
    public:
        VLine() : Line(VERT) {}
    };
}
