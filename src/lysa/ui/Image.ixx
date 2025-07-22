/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.image;

import lysa.math;
import lysa.ui.rect;
import lysa.ui.widget;
import lysa.resources.image;

export namespace lysa::ui {

    /**
     * %A widget to display a 2D image
     */
    class Image : public Widget {
    public:
        Image(const std::shared_ptr<lysa::Image>& image = nullptr, bool autoSize = true);

        void setAutoSize(bool autoSize);

        void setColor(const float4& color);

        auto getColor() const { return color; }

        void setImage(const std::shared_ptr<lysa::Image>& image);

        void _setSize(float width, float height) override;

        auto getImage() const { return image; }

    private:
        float4 color{1.0f};
        bool autoSize;
        std::shared_ptr<lysa::Image> image;

        void autoResize();

        void eventCreate() override;
    };
}
