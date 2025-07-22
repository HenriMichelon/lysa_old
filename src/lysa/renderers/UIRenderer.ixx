/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
export module lysa.renderers.ui;

import std;
import vireo;
import lysa.configuration;
import lysa.math;
import lysa.renderers.vector;
import lysa.resources.font;
import lysa.resources.image;
import lysa.ui.rect;

export namespace lysa {

    class UIRenderer : public VectorRenderer {
    public:
        UIRenderer(const RenderingConfiguration& renderingConfiguration);

        void resize(const vireo::Extent& extent);

        auto getAspectRatio() const { return vectorRatio; }

        // auto getExtent() const { return vectorExtent; }

        // Draw a 1-fragment width line
        void drawLine(const float2& start, const float2& end);

        // Draw a filled rectangle
        void drawFilledRect(const ui::Rect &rect, float clipWidth, float clipHeight);

        // Draw a filled rectangle with an image
        void drawFilledRect(
            const ui::Rect &rect,
            float clipWidth,
            float clipHeight,
            const std::shared_ptr<Image> &texture);

        // Draw a filled rectangle
        void drawFilledRect(
            float x, float y,
            float w, float h,
            float clipWidth,
            float clipHeight,
            const std::shared_ptr<Image> &texture);

        // Draw a rectangle filled with a text
        void drawText(
            const std::string& text,
            Font& font,
            const ui::Rect& rect,
            float clipWidth,
            float clipHeight);

        // Draw a rectangle filled with a text
        void drawText(
            const std::string& text,
            Font& font,
            float x, float y,
            float w, float  h,
            float clipWidth, float clipHeight);

        // Change the color of the fragments for the next drawing commands
        auto setPenColor(const float4& color) { penColor = color; }

        // Change the [x,y] translation for the next drawing commands
        auto setTranslate(const float2& t) { translate = t; }

        // Change the global transparency for the next drawing commands. Value is subtracted from the vertex alpha
        auto setTransparency(const float a) { transparency = a; }

    private:
        struct PushConstants {
            int   textureIndex;
            float clipX;
            float clipY;
        };

        // Fragment color for the next drawing commands
        float4 penColor{1.0f, 1.0f, 1.0f, 1.0f};
        // [x,y] translation for the next drawing commands
        float2 translate{0.0f, 0.0f};
        // Global transparency for the next drawing commands. Value is subtracted from the vertex alpha
        float transparency{0.0f};

        // float2 vectorExtent{};
        float vectorRatio{};
    };
}