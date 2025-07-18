/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.ui;

namespace lysa {

    UIRenderer::UIRenderer(
        const RenderingConfiguration& renderingConfiguration) :
        VectorRenderer{false, renderingConfiguration, L"UI Renderer"} {
    }

    void UIRenderer::drawLine(const float2& start, const float2& end) {
        const auto scaledStart = (start + translate) / vectorExtent;
        const auto scaledEnd = (end + translate) / vectorExtent;
        const auto alpha = std::max(0.0f, static_cast<float>(penColor.a - transparency));
        const auto color = float4{penColor.rgb, alpha};
        linesVertices.push_back( {{scaledStart, 0.0f}, {}, color });
        linesVertices.push_back( {{scaledEnd, 0.0f}, {}, color });
        vertexBufferDirty = true;
    }

    void UIRenderer::drawFilledRect(const ui::Rect &rect, const float clipWidth, const float clipHeight) {
        drawFilledRect(
            rect.x, rect.y,
            rect.width, rect.height,
            clipWidth, clipHeight,
            nullptr);
    }

    void UIRenderer::drawFilledRect(
           const ui::Rect &rect,
           const float clipWidth,
           const float clipHeight,
           const std::shared_ptr<Image> &texture) {
        drawFilledRect(
            rect.x, rect.y,
            rect.width, rect.height,
            clipWidth, clipHeight,
            texture);
    }

    void UIRenderer::drawFilledRect(
            const float x, const float y,
            const float w, const float h,
            const float clipWidth, const float clipHeight,
            const std::shared_ptr<Image> &texture) {
        const auto pos  = (float2{x, y} + translate) / vectorExtent;
        const float2 size = float2{w, h} / vectorExtent;
        const auto color = float4{penColor.rgb, std::max(0.0f, static_cast<float>(penColor.a - transparency))};
        /*
         * v1 ---- v3
         * |  \     |
         * |    \   |
         * v0 ---- v2
         */
        const auto v0= float3{pos.x, pos.y, 0.0f};
        const auto v1 = float3{pos.x, pos.y + size.y, 0.0f};
        const auto v2 = float3{pos.x + size.x, pos.y, 0.0f};
        const auto v3 = float3{pos.x + size.x, pos.y + size.y, 0.0f};
        const auto uvClip = float2{clipWidth / w, clipHeight / h};

        triangleVertices.push_back( {v0, {0.0f, 1.0f}, color, uvClip });
        triangleVertices.push_back( {v1, {0.0f, 0.0f}, color, uvClip });
        triangleVertices.push_back( {v2, {1.0f, 1.0f}, color, uvClip });

        triangleVertices.push_back( {v1, {0.0f, 0.0f}, color, uvClip });
        triangleVertices.push_back( {v3, {1.0f, 0.0f}, color, uvClip });
        triangleVertices.push_back( {v2, {1.0f, 1.0f}, color, uvClip });

        vertexBufferDirty = true;
    }

    void UIRenderer::resize(const vireo::Extent& extent) {
        vectorExtent = {(extent.width * 1000.0f) / extent.height, 1000.0f};
        vectorRatio = vectorExtent.x / vectorExtent.y;
    }
}