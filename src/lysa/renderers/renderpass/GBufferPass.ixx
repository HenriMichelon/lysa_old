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

        // auto getMaterialBuffer(const uint32 frameIndex) const { return framesData[frameIndex].materialBuffer; }

    private:
        const std::wstring DEFAULT_VERTEX_SHADER{L"default.vert"};
        const std::wstring DEFAULT_FRAGMENT_SHADER{L"gbuffers.frag"};

        static constexpr int BUFFER_POSITION{0};
        static constexpr int BUFFER_NORMAL{1};
        static constexpr int BUFFER_ALBEDO{2};
        static constexpr int BUFFER_MATERIAL{3};

        struct FrameData {
            std::shared_ptr<vireo::RenderTarget>  positionBuffer;
            std::shared_ptr<vireo::RenderTarget>  normalBuffer;
            std::shared_ptr<vireo::RenderTarget>  albedoBuffer;
            // std::shared_ptr<vireo::RenderTarget>  materialBuffer;
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorRenderFormats  = {
                vireo::ImageFormat::R16G16B16A16_SFLOAT, // RGB: Position, A: unsued (metallic ?)
                vireo::ImageFormat::R16G16B16A16_SFLOAT, // RGB: Normal, A: unused (roughness ?)
                vireo::ImageFormat::R8G8B8A8_UNORM,      // RGB: Albedo, A: unused (ao ?)
                //vireo::ImageFormat::R8G8B8A8_UNORM,      // RGBA: Material index
            },
            .colorBlendDesc      = {
                {}, // Position
                {}, // Normal
                {}, // Albedo
                // {} // Material
            },
            .cullMode            = vireo::CullMode::BACK,
            .depthTestEnable     = true,
            .depthWriteEnable    = false,
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {
                { .clear = true }, // Position
                { .clear = true }, // Normal
                { .clear = true }, // Albedo
                // { .clear = true }, // Material
            },
            .depthTestEnable    = pipelineConfig.depthTestEnable,
            .stencilTestEnable  = pipelineConfig.stencilTestEnable,
        };

        int buffersResized{0};
        std::vector<FrameData> framesData;
        std::unordered_map<pipeline_id, std::shared_ptr<vireo::GraphicPipeline>> pipelines;
    };
}