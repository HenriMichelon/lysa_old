/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.gbuffer_pass;

import std;
import vireo;
import lysa.configuration;
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderpass;

export namespace lysa {

    class GBufferPass : public Renderpass {
    public:
        GBufferPass(const RenderingConfiguration& config);

        void updatePipelines(
            const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds);

        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex);

        void resize(const vireo::Extent& extent) override;

        auto getPositionBuffer(const uint32 frameIndex) const {
            return framesData[frameIndex].positionBuffer;
        }

        auto getNormalBuffer(const uint32 frameIndex) const {
            return framesData[frameIndex].normalBuffer;
        }

        auto getAlbedoBuffer(const uint32 frameIndex) const {
            return framesData[frameIndex].albedoBuffer;
        }

        auto getEmissiveBuffer(const uint32 frameIndex) const {
            return framesData[frameIndex].emissiveBuffer;
        }

    private:
        const std::wstring VERTEX_SHADER{L"default.vert"};
        const std::wstring FRAGMENT_SHADER{L"gbuffers.frag"};

        static constexpr int BUFFER_POSITION{0};
        static constexpr int BUFFER_NORMAL{1};
        static constexpr int BUFFER_ALBEDO{2};
        static constexpr int BUFFER_EMISSIVE{3};

        struct FrameData {
            std::shared_ptr<vireo::RenderTarget>  positionBuffer;
            std::shared_ptr<vireo::RenderTarget>  normalBuffer;
            std::shared_ptr<vireo::RenderTarget>  albedoBuffer;
            std::shared_ptr<vireo::RenderTarget>  emissiveBuffer;
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorRenderFormats  = {
                vireo::ImageFormat::R16G16B16A16_SFLOAT, // RGB: Position, A: metallic
                vireo::ImageFormat::R16G16B16A16_SFLOAT, // RGB: Normal, A: roughness
                vireo::ImageFormat::R8G8B8A8_UNORM,      // RGB: Albedo, A: unused (ao ?)
                vireo::ImageFormat::R8G8B8A8_UNORM,      // RGB: emissive color, A: have material+roughness?
            },
            .colorBlendDesc      = {
                {}, // Position
                {}, // Normal
                {}, // Albedo
                {}  // Emissive
            },
            .cullMode            = vireo::CullMode::BACK,
            .depthTestEnable     = true,
            .depthWriteEnable    = true,
            .stencilTestEnable   = true,
            .frontStencilOpState = {
                .failOp      = vireo::StencilOp::KEEP,
                .passOp      = vireo::StencilOp::REPLACE,
                .depthFailOp = vireo::StencilOp::KEEP,
                .compareOp   = vireo::CompareOp::ALWAYS,
                .compareMask = 0xff,
                .writeMask   = 0xff
            }
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {
                { .clear = true }, // Position
                { .clear = true }, // Normal
                { .clear = true }, // Albedo
                { .clear = true }, // Emissive
            },
            .depthTestEnable    = pipelineConfig.depthTestEnable,
            .stencilTestEnable  = pipelineConfig.stencilTestEnable,
        };

        int buffersResized{0};
        std::vector<FrameData> framesData;
        std::unordered_map<pipeline_id, std::shared_ptr<vireo::GraphicPipeline>> pipelines;
    };
}