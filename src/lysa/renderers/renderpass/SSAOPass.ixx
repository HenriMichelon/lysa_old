/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.ssao_pass;

import std;
import vireo;
import lysa.configuration;
import lysa.math;
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderpass;
import lysa.renderers.renderpass.gbuffer_pass;

export namespace lysa {

    class SSAOPass : public Renderpass {
    public:
        SSAOPass(const RenderingConfiguration& config, const GBufferPass& gBufferPass);

        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            uint32 frameIndex);

        void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) override;

        auto getSSAOColorBuffer(const uint32 frameIndex) const {
            return framesData[frameIndex].ssaoColorBuffer;
        }

        auto getSSAOBufferFormat() const {
            return pipelineConfig.colorRenderFormats[0];
        }

    private:
        const std::wstring VERTEX_SHADER{L"quad.vert"};
        const std::wstring FRAGMENT_SHADER{L"ssao.frag"};

        static constexpr vireo::DescriptorIndex BINDING_PARAMS{0};
        static constexpr vireo::DescriptorIndex BINDING_POSITION_BUFFER{1};
        static constexpr vireo::DescriptorIndex BINDING_NORMAL_BUFFER{2};
        static constexpr vireo::DescriptorIndex BINDING_NOISE_TEXTURE{3};

        struct Params {
            float2 screenSize;
            float2 noiseScale;
            float radius{0.5f};
            float bias{0.025f};
            float power{1.2f};
            uint sampleCount{64};
            float4 samples[64];
        };

        struct FrameData {
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;
            std::shared_ptr<vireo::RenderTarget> ssaoColorBuffer;
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorRenderFormats = { vireo::ImageFormat::R8_UNORM },
            .colorBlendDesc = {{}},
            .stencilTestEnable = true,
            .frontStencilOpState = {
                .failOp = vireo::StencilOp::KEEP,
                .passOp = vireo::StencilOp::KEEP,
                .depthFailOp = vireo::StencilOp::KEEP,
                .compareOp = vireo::CompareOp::EQUAL,
                .compareMask = 0xff,
                .writeMask = 0x00
            }
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{ }},
            .depthTestEnable    = pipelineConfig.depthTestEnable,
            .stencilTestEnable  = pipelineConfig.stencilTestEnable,
        };

        Params params;
        const GBufferPass& gBufferPass;
        std::vector<FrameData> framesData;
        std::shared_ptr<vireo::Buffer> paramsBuffer;
        std::shared_ptr<vireo::Image> noiseTexture;
        std::shared_ptr<vireo::GraphicPipeline> pipeline;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
    };
}