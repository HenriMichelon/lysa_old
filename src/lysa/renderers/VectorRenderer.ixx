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

    class VectorRenderer {
    public:
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

        void drawLine(const float3& from, const float3& to, const float4& color);

        void drawTriangle(const float3& v1, const float3& v2, const float3& v3, const float4& color);

        void drawText(
            const std::string& text,
            Font& font,
            float fontScale,
            const float3& position,
            const float4& color);

        void restart();

        void update(
            const vireo::CommandList& commandList,
            uint32 frameIndex);

        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            uint32 frameIndex);

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
            alignas(16) float2 uvClip;
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
            {"CLIP", vireo::AttributeFormat::R32G32_FLOAT, offsetof(Vertex, uvClip)},
            {"TEXTURE", vireo::AttributeFormat::R32_SINT, offsetof(Vertex, textureIndex)},
            {"FONT", vireo::AttributeFormat::R32_SINT, offsetof(Vertex, fontIndex)},
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc = {{ }},
            .cullMode = vireo::CullMode::NONE,
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