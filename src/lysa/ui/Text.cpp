/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.text;

import lysa.ui.window;
import lysa.ui.window_manager;

namespace lysa::ui {

    Text::Text(const std::string& text) : Widget(TEXT), text(std::move(text)) {
        allowChildren  = false;
        drawBackground = false;
    }

    void Text::setText(const std::string & text) {
        this->text = text;
        if (parent) {
            parent->refresh();
        }
        float w, h;
        getSize(w, h);
        _setSize(w, h);
        if (!parent) {
            refresh();
        }
    }

    void Text::setTextColor(const float4 &c) {
        textColor = c;
        refresh();
    }

    void Text::getSize(float &width, float &height) {
        getFont().getSize(text, width, height);
    }

    void Text::_setSize(const float width, const float height) {
        if (!window) { return; }
        if (width == 0 && height == 0 && rect.width == 0 && rect.height == 0) {
            float w, h;
            getSize(w, h);
            const auto ratio = window->getWindowManager().getRenderer().getAspectRatio();
            _setSize(w / ratio, h);
        } else {
            Widget::_setSize(width, height);
        }
    }

    Rect Text::_getDefaultRect() {
        if (rect.width == 0 && rect.height == 0) {
            float w, h;
            getSize(w, h);
            _setSize(w, h);
        }
        return Widget::_getDefaultRect();
    }

    void Text::eventCreate() {
        if (all(textColor == float4{0.0f})) {
            textColor = static_cast<Window *>(window)->getDefaultTextColor();
        }
        const auto ratio = window->getWindowManager().getRenderer().getAspectRatio();
        getSize(rect.width, rect.height);
        rect.width /= ratio;
        Widget::eventCreate();
    }

}
