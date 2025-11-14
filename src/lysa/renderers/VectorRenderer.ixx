/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <cstdlib>
export module lysa.renderers.vector;

import vireo;
import lysa.configuration;
import lysa.math;
import lysa.scene;
import lysa.types;
import lysa.resources.font;
import lysa.resources.image;

export namespace lysa {

    /**
     * Immediate-style 2D/3D vector renderer used for debug and UI primitives.
     *  - Accumulate simple geometric primitives (lines, triangles, glyph quads)
     *    into CPU-side vertex arrays and stream them to the GPU when required.
     *  - Provide minimal state (colors, textures, fonts) and pipelines to draw
     *    primitive batches either in screen space or with a camera.
     *  - Offer helpers to render text using bitmap fonts.
     *
     * Notes:
     *  - Thread-safety: calls are expected from the render thread only.
     *  - Call restart() between frames to clear accumulated geometry.
     */
    class VectorRenderer {
    public:
        /**
         * Constructs a vector renderer.
         * @param depthTestEnable    Enable depth test for 3D primitives.
         * @param enableAlphaBlending Enable alpha blending (for UI/text).
         * @param useTextures        Allow textured triangles.
         * @param renderingConfiguration Global rendering config.
         * @param name               Debug name for pipelines.
         * @param shadersName        Base shader name for vector primitives.
         * @param glyphShadersName   Base shader name for glyph rendering.
         * @param filledTriangles    True to use triangle pipeline; false for wireframe only.
         * @param useCamera          True to apply Scene view/projection.
         */
        VectorRenderer(
            bool depthTestEnable,
            bool enableAlphaBlending,
            bool useTextures,
            const RenderingConfiguration& renderingConfiguration,
            const std::string& name = "VectorRenderer",
            const std::string& shadersName = "vector",
            const std::string& glyphShadersName = "glyph",
            bool filledTriangles = false,
            bool useCamera = true);

        /** Adds a colored line segment to the current batch. */
        void drawLine(const float3& from, const float3& to, const float4& color);

        /** Adds a filled triangle to the current batch. */
        void drawTriangle(const float3& v1, const float3& v2, const float3& v3, const float4& color);

        /**
         * Adds text to the current batch using the provided font.
         * @param text       UTF-8 string to render.
         * @param font       Font object providing glyph atlas/metrics.
         * @param fontScale  Scaling factor applied to glyph metrics.
         * @param position   Baseline origin for the text in world/screen space.
         * @param rotation   Orientation for the text in world space.
         * @param innerColor Color applied to glyphs (vertex alpha can modulate it).
         */
        void drawText(
            const std::string& text,
            Font& font,
            float fontScale,
            const float3& position,
            const quaternion& rotation,
            const float4& innerColor);

        /** Clears accumulated geometry (call once per frame). */
        void restart();

        /** Uploads dirty vertex buffers and prepares descriptor sets. */
        void update(
            const vireo::CommandList& commandList,
            uint32 frameIndex);

        /**
         * Renders accumulated primitives using the given Scene (with camera).
         * @param commandList     Command buffer to record into.
         * @param scene           Scene providing camera matrices.
         * @param colorAttachment Target color surface.
         * @param depthAttachment Target depth surface (may be null if no depth).
         * @param frameIndex      Index of the current frame in flight.
         */
        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            uint32 frameIndex);

        /**
         * Renders accumulated primitives without referencing a Scene camera.
         * Useful for UI-like overlays in screen space.
         */
        void render(
            vireo::CommandList& commandList,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            uint32 frameIndex);

        virtual ~VectorRenderer() = default;
        VectorRenderer(VectorRenderer&) = delete;
        VectorRenderer& operator=(VectorRenderer&) = delete;

    protected:
        struct Vertex {
            alignas(16) float3 position;
            alignas(16) float2 uv;
            alignas(16) float4 color;
            alignas(16) int textureIndex{-1};
            alignas(16) int fontIndex{-1};
        };

        const RenderingConfiguration& config;
        // Vertex buffer needs to be re-uploaded to GPU
        bool vertexBufferDirty{true};
        // All the vertices for lines
        std::vector<Vertex> linesVertices;
        // All the vertices for triangles
        std::vector<Vertex> triangleVertices;
        // All the vertices for the texts
        std::vector<Vertex> glyphVertices;

        int32 addTexture(const std::shared_ptr<Image> &texture);

        int32 addFont(const Font &font);

    private:
        static constexpr auto MAX_TEXTURES{100};
        static constexpr auto MAX_FONTS{10};

        static constexpr vireo::DescriptorIndex FONT_PARAMS_BINDING{0};
        static constexpr vireo::DescriptorIndex FONT_PARAMS_SET{2};

        const std::string name;
        const bool useCamera;
        const bool useTextures;
        std::shared_ptr<vireo::Image> blankImage;

        vireo::DescriptorIndex globalUniformIndex;
        vireo::DescriptorIndex texturesIndex;

        struct GlobalUniform {
            float4x4 projection{1.0f};
            float4x4 view{1.0f};
        };

        struct FrameData {
            std::shared_ptr<vireo::Buffer> globalUniform;
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        };

        const std::vector<vireo::VertexAttributeDesc> vertexAttributes{
            {"POSITION", vireo::AttributeFormat::R32G32B32_FLOAT, offsetof(Vertex, position)},
            {"TEXCOORD", vireo::AttributeFormat::R32G32_FLOAT, offsetof(Vertex, uv)},
            {"COLOR", vireo::AttributeFormat::R32G32B32A32_FLOAT, offsetof(Vertex, color)},
            {"TEXTURE", vireo::AttributeFormat::R32_SINT, offsetof(Vertex, textureIndex)},
            {"FONT", vireo::AttributeFormat::R32_SINT, offsetof(Vertex, fontIndex)},
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc = {{ }},
            .cullMode = vireo::CullMode::NONE,
        };

        vireo::GraphicPipelineConfiguration glyphPipelineConfig {
            .colorBlendDesc = {
            {
                    .blendEnable = true,
                    .srcColorBlendFactor = vireo::BlendFactor::SRC_ALPHA,
                    .dstColorBlendFactor = vireo::BlendFactor::ONE_MINUS_SRC_ALPHA,
                    .colorBlendOp = vireo::BlendOp::ADD,
                    .srcAlphaBlendFactor = vireo::BlendFactor::ONE,
                    .dstAlphaBlendFactor = vireo::BlendFactor::ONE_MINUS_SRC_ALPHA,
                    .alphaBlendOp = vireo::BlendOp::ADD,
                    .colorWriteMask = vireo::ColorWriteMask::ALL,
                }},
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{ }},
        };

        vireo::Extent currentExtent{};
        std::vector<FrameData> framesData;

        // Number of vertices for the currently allocated buffers, used to check if we need to resize the buffers
        uint32 vertexCount{0};
        // Staging vertex buffer used when updating GPU memory
        std::shared_ptr<vireo::Buffer> stagingBuffer;
        // Vertex buffer in GPU memory
        std::shared_ptr<vireo::Buffer> vertexBuffer;
        // Used when we need to postpone the buffer destruction when they are in use by another frame in flight
        std::list<std::shared_ptr<vireo::Buffer>> oldBuffers;

        std::vector<std::shared_ptr<vireo::Image>> textures;
        // Indices of each image in the descriptor binding
        std::map<unique_id, int32> texturesIndices{};

        std::vector<FontParams> fontsParams{};
        std::shared_ptr<vireo::Buffer> fontsParamsUniform;
        std::shared_ptr<vireo::DescriptorSet> fontDescriptorSet;
        std::shared_ptr<vireo::DescriptorLayout> fontDescriptorLayout;
        std::map<unique_id, int32> fontsIndices{};

        std::shared_ptr<vireo::GraphicPipeline>  pipelineLines;
        std::shared_ptr<vireo::GraphicPipeline>  pipelineTriangles;
        std::shared_ptr<vireo::GraphicPipeline>  pipelineGlyphs;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
    };
}