/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.transparency_pass;

import std;
import vireo;
import lysa.configuration;
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderpass;

export namespace lysa {

    class TransparencyPass : public Renderpass {
    public:
        TransparencyPass(const RenderingConfiguration& config);

        void updatePipelines(
           const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds);

        void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) override;

        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex);

    private:
        const std::wstring VERTEX_SHADER_OIT{L"default.vert"};
        const std::wstring FRAGMENT_SHADER_OIT{L"transparency_oit.frag"};
        const std::wstring VERTEX_SHADER_COMPOSITE{L"quad.vert"};
        const std::wstring FRAGMENT_SHADER_COMPOSITE{L"transparency_oit_composite.frag"};

        static constexpr vireo::DescriptorIndex BINDING_ACCUM_BUFFER{0};
        static constexpr vireo::DescriptorIndex BINDING_REVEALAGE_BUFFER{1};

        struct FrameData {
            std::shared_ptr<vireo::DescriptorSet> compositeDescriptorSet;
            std::shared_ptr<vireo::RenderTarget>  accumBuffer;
            std::shared_ptr<vireo::RenderTarget>  revealageBuffer;
        };

        vireo::GraphicPipelineConfiguration oitPipelineConfig {
            .colorRenderFormats  = {
                vireo::ImageFormat::R16G16B16A16_SFLOAT, // Color accumulation
                vireo::ImageFormat::R16_SFLOAT,          // Alpha accumulation
            },
            .colorBlendDesc = {
                {
                    .blendEnable = true,
                    .srcColorBlendFactor = vireo::BlendFactor::ONE,
                    .dstColorBlendFactor = vireo::BlendFactor::ONE,
                    .colorBlendOp = vireo::BlendOp::ADD,
                    .srcAlphaBlendFactor = vireo::BlendFactor::ONE,
                    .dstAlphaBlendFactor = vireo::BlendFactor::ONE,
                    .alphaBlendOp = vireo::BlendOp::ADD,
                    .colorWriteMask = vireo::ColorWriteMask::ALL,
                },
                {
                    .blendEnable = true,
                    .srcColorBlendFactor = vireo::BlendFactor::ZERO,
                    .dstColorBlendFactor = vireo::BlendFactor::ONE_MINUS_SRC_COLOR,
                    .colorBlendOp = vireo::BlendOp::ADD,
                    .srcAlphaBlendFactor = vireo::BlendFactor::ONE,
                    .dstAlphaBlendFactor = vireo::BlendFactor::ONE,
                    .alphaBlendOp = vireo::BlendOp::ADD,
                    .colorWriteMask = vireo::ColorWriteMask::RED,
                }},
            .depthTestEnable = true,
            .depthWriteEnable = false
        };

        vireo::RenderingConfiguration oitRenderingConfig {
            .colorRenderTargets = {
                {
                    .clear = true,
                    .clearValue = {0.0f, 0.0f, 0.0f, 0.0f},
                },
                {
                    .clear = true,
                    .clearValue = {1.0f, 0.0f, 0.0f, 0.0f},
                }
            },
            .depthTestEnable = oitPipelineConfig.depthTestEnable,
        };

        vireo::GraphicPipelineConfiguration compositePipelineConfig {
            .colorBlendDesc = {
            {
                    .blendEnable = true,
                    .srcColorBlendFactor = vireo::BlendFactor::ONE_MINUS_SRC_ALPHA,
                    .dstColorBlendFactor = vireo::BlendFactor::SRC_ALPHA,
                    .colorBlendOp = vireo::BlendOp::ADD,
                    .srcAlphaBlendFactor = vireo::BlendFactor::ONE,
                    .dstAlphaBlendFactor = vireo::BlendFactor::ZERO,
                    .alphaBlendOp = vireo::BlendOp::ADD,
                    .colorWriteMask = vireo::ColorWriteMask::ALL,
                }
            },
            .depthTestEnable = false,
            .depthWriteEnable = false
        };

        vireo::RenderingConfiguration compositeRenderingConfig {
            .colorRenderTargets = {{}},
            .depthTestEnable = compositePipelineConfig.depthTestEnable,
        };

        std::vector<FrameData> framesData;
        std::shared_ptr<vireo::Pipeline> compositePipeline;
        std::shared_ptr<vireo::DescriptorLayout> compositeDescriptorLayout;
        std::unordered_map<pipeline_id, std::shared_ptr<vireo::GraphicPipeline>> oitPipelines;
    };
}