/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <hb.h>
#include <hb-ft.h>
#include FT_FREETYPE_H
export module lysa.resources.font;

import std;
import lysa.resources.image;
import lysa.resources.resource;
import lysa.constants;
import lysa.math;
import lysa.types;

export namespace lysa {

    struct FontParams {
        float2 pxRange{FLOAT2ZERO};
        float4 outlineColor{0.0f, 0.0f, 0.0f, 1.0f};
        float threshold{0.5f};
        float outlineBias{1.0f/4.0f};
        float outlineWidthAbsolute{1.0f/3.0f};
        float outlineWidthRelative{1.0f/20.0f};
        float outlineBlur{0.0f};
        float gamma{1.0f};
    };

    /**
     * %A font resource to render text
     * %A font is a combination of a font file name and a size.
     */
    class Font : public Resource {
    public:
        friend class Window;

        struct GlyphBounds {
            float left{0.0f};
            float bottom{0.0f};
            float right{0.0f};
            float top{0.0f};
        };

        struct GlyphInfo {
            uint32 index{0};
            float advance{0.0f};
            GlyphBounds planeBounds{};
            float2 uv0{0.0f};
            float2 uv1{0.0f};
        };

        /**
         * Creates a font resource
         * @param path : font file path, relative to the application working directory
         */
        Font(const std::string &path);

        ~Font() override;

        /**
         * Returns the size (in pixels) for a string.
         */
        void getSize(const std::string &text, float scale, float &width, float &height);

        /**
         * Returns the font size in the atlas
         */
        auto getFontSize() const { return size; }

        //Relative to the font size
        auto getLineHeight() const { return lineHeight; }

        const GlyphInfo& getGlyphInfo(uint32 index) const;

        auto getAtlas() const { return atlas; }

        const auto& getFontParams() const { return params; }

        void setOutlineColor(const float4 &color) {
            params.outlineColor = color;
        }

        void setOutlineBias(const float bias) {
            params.outlineBias = bias;
        }

        void setOutlineWidthAbsolute(const float width) {
            params.outlineWidthAbsolute = width;
        }

        void setOutlineWidthRelative(const float width) {
            params.outlineWidthRelative = width;
        }

        void setOutlineBlur(const float blur) {
            params.outlineBlur = blur;
        }

        void setOutlineThreshold(const float threshold) {
            params.threshold = threshold;
        }

        auto getHarfBuzzFont() const { return hbFont; }

    private:
        uint32 size;
        float ascender;
        float descender;
        float lineHeight;
        FontParams params;
        std::shared_ptr<Image> atlas;
        std::unordered_map<uint32, GlyphInfo> glyphs;

        static FT_Library ftLibrary;
        FT_Face ftFace{nullptr};
        hb_font_t* hbFont{nullptr};
    };

}
