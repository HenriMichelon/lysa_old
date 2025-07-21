/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.image;

import lysa.window;

namespace lysa::ui {

    Image::Image(const std::shared_ptr<lysa::Image> &image, const bool autoSize) : Widget{IMAGE}, autoSize{autoSize} {
        setImage(image);
    }

    void Image::_setSize(const float width, const float height) {
        if (autoSize) { return; }
        if (width == 0 && height == 0 && rect.width == 0 && rect.height == 0) {
            const auto ratio = static_cast<Window*>(window)->getAspectRatio();
            Widget::_setSize(std::round(width / ratio), std::round(height / 1.0f));
        }
        else {
            Widget::_setSize(width, height);
        }
    }

    void Image::autoResize() {
        const auto &ratio = static_cast<Window*>(window)->getAspectRatio();
        Widget::_setSize(std::round(image->getWidth() / ratio), std::round(image->getHeight() / 1.0f));
    }

    void Image::setColor(const float4 &color) {
        this->color = color;
        refresh();
    }


    void Image::setAutoSize(const bool autoSize) {
        if (autoSize == this->autoSize) {
            return;
        }
        this->autoSize = autoSize;
        if (autoSize && image) {
            this->autoResize();
        }
    }

    void Image::setImage(const std::shared_ptr<lysa::Image> &image) {
        if (this->image == image) {
            return;
        }
        this->image = image;
        if (image) {
            if (autoSize) {
                autoResize();
            } else {
                refresh();
            }
        }
    }

}
