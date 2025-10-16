/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <hb.h>
module lysa.renderers.ui;

import lysa.constants;

namespace lysa {

    UIRenderer::UIRenderer(
        const RenderingConfiguration& renderingConfiguration) :
        VectorRenderer{
            false,
            true,
            true,
            renderingConfiguration,
            "UI Renderer",
            "vector_ui",
            "glyph_ui",
            true,
            false} {
    }

    void UIRenderer::drawLine(const float2& start, const float2& end) {
        const auto scaledStart = (start + translate) / VECTOR_SCREEN_SIZE;
        const auto scaledEnd = (end + translate) / VECTOR_SCREEN_SIZE;
        const auto alpha = std::max(0.0f, static_cast<float>(penColor.a - transparency));
        const auto color = float4{penColor.rgb, alpha};
        linesVertices.push_back( {{scaledStart, 0.0f}, {}, color, -1 });
        linesVertices.push_back( {{scaledEnd, 0.0f}, {}, color, -1 });
        vertexBufferDirty = true;
    }

    void UIRenderer::drawFilledRect(const ui::Rect &rect) {
        drawFilledRect(
            rect.x, rect.y,
            rect.width, rect.height,
            nullptr);
    }

    void UIRenderer::drawFilledRect(
           const ui::Rect &rect,
           const std::shared_ptr<Image> &texture) {
        drawFilledRect(
            rect.x, rect.y,
            rect.width, rect.height,
            texture);
    }

    void UIRenderer::drawFilledRect(
            const float x, const float y,
            const float w, const float h,
            const std::shared_ptr<Image> &texture) {
        const auto pos  = (float2{x, y} + translate) / VECTOR_SCREEN_SIZE;
        const float2 size = float2{w, h} / VECTOR_SCREEN_SIZE;
        const auto color = float4{penColor.rgb, std::max(0.0f, static_cast<float>(penColor.a - transparency))};
        /*
         * v1 ---- v3
         * |  \     |
         * |    \   |
         * v0 ---- v2
         */
        const auto v0 = float3{pos.x, pos.y, 0.0f};
        const auto v1 = float3{pos.x, pos.y + size.y, 0.0f};
        const auto v2 = float3{pos.x + size.x, pos.y, 0.0f};
        const auto v3 = float3{pos.x + size.x, pos.y + size.y, 0.0f};

        auto textureIndex{-1};
        if (texture) {
            textureIndex = addTexture(texture);
        }

        triangleVertices.push_back( {v0, {0.0f, 1.0f}, color, textureIndex });
        triangleVertices.push_back( {v1, {0.0f, 0.0f}, color, textureIndex });
        triangleVertices.push_back( {v2, {1.0f, 1.0f}, color, textureIndex });
        triangleVertices.push_back( {v1, {0.0f, 0.0f}, color, textureIndex });
        triangleVertices.push_back( {v3, {1.0f, 0.0f}, color, textureIndex });
        triangleVertices.push_back( {v2, {1.0f, 1.0f}, color, textureIndex });
        vertexBufferDirty = true;
    }

    void UIRenderer::drawText(
        const std::string& text,
        Font& font,
        const float fontScale,
        const float x,
        const float y) {
        float2 pos  = (float2{x, y} + translate) / VECTOR_SCREEN_SIZE;
        const auto scale = fontScale * font.getFontSize() / VECTOR_SCREEN_SIZE;
        const auto textureIndex = addTexture(font.getAtlas());
        const auto fontIndex = addFont(font);
        const auto innerColor = float4{penColor.rgb, std::max(0.0f, static_cast<float>(penColor.a - transparency))};

        hb_buffer_t* hb_buffer = hb_buffer_create();
        hb_buffer_add_utf8(hb_buffer, text.c_str(), -1, 0, -1);
        hb_buffer_guess_segment_properties(hb_buffer);
        hb_shape(font.getHarfBuzzFont(), hb_buffer, nullptr, 0);
        unsigned int glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
        for (unsigned int i = 0; i < glyph_count; i++) {
            auto glyphInfo = font.getGlyphInfo(glyph_info[i].codepoint);
            auto plane = Font::GlyphBounds{};
            plane.left = scale * glyphInfo.planeBounds.left ;
            plane.right = scale * glyphInfo.planeBounds.right;
            plane.top = scale * glyphInfo.planeBounds.top;
            plane.bottom = scale * glyphInfo.planeBounds.bottom;
            /*
            * v1 ---- v3
            * |  \     |
            * |    \   |
            * v0 ---- v2
            */
            const float3 v0 = { pos.x + plane.left,  pos.y + plane.bottom, 0.0f };
            const float3 v1 = { pos.x + plane.left,  pos.y + plane.top, 0.0f };
            const float3 v2 = { pos.x + plane.right, pos.y + plane.bottom, 0.0f };
            const float3 v3 = { pos.x + plane.right, pos.y + plane.top, 0.0f };
            glyphVertices.push_back({v0, {glyphInfo.uv0.x, glyphInfo.uv1.y}, innerColor, textureIndex, fontIndex});
            glyphVertices.push_back({v1, {glyphInfo.uv0.x, glyphInfo.uv0.y}, innerColor, textureIndex, fontIndex});
            glyphVertices.push_back({v2, {glyphInfo.uv1.x, glyphInfo.uv1.y}, innerColor, textureIndex, fontIndex});
            glyphVertices.push_back({v1, {glyphInfo.uv0.x, glyphInfo.uv0.y}, innerColor, textureIndex, fontIndex});
            glyphVertices.push_back({v3, {glyphInfo.uv1.x, glyphInfo.uv0.y}, innerColor, textureIndex, fontIndex});
            glyphVertices.push_back({v2, {glyphInfo.uv1.x, glyphInfo.uv1.y}, innerColor, textureIndex, fontIndex});
            pos.x += scale * glyphInfo.advance;
        }
        hb_buffer_destroy(hb_buffer);
        vertexBufferDirty = true;
    }

    void UIRenderer::resize(const vireo::Extent& extent) {
        vectorRatio = static_cast<float>(extent.width) / extent.height;
    }
}