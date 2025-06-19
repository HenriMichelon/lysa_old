/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.depth_prepass;

import std;
import vireo;
import lysa.configuration;
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderpass;

export namespace lysa {
    class DepthPrepass : public Renderpass {
    public:
        DepthPrepass(const RenderingConfiguration& config, bool withStencil);

        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment);

        void updatePipelines(const std::unordered_map<uint32, std::vector<std::shared_ptr<Material>>>& pipelineIds);

    private:
        vireo::GraphicPipelineConfiguration pipelineConfig {
            .cullMode            = vireo::CullMode::BACK,
            .depthTestEnable     = true,
            .depthWriteEnable    = true,
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
            .depthTestEnable    = pipelineConfig.depthTestEnable,
            .clearDepthStencil = true,
        };

        std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>> pipelines;
    };
}