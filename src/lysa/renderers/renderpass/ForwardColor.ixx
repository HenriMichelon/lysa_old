/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.forward_color;

import std;
import vireo;
import lysa.configuration;
import lysa.renderers.renderpass;
import lysa.renderers.samplers;
import lysa.renderers.scene_data;

export namespace lysa {
    class ForwardColor : public Renderpass {
    public:
        ForwardColor(
            const RenderingConfig& config,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const Samplers& samplers);

        void render(
            uint32 frameIndex,
            SceneData& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::CommandList>& commandList,
            bool recordLastBarrier) override;

    private:
        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc   = { { .blendEnable = true } },
            .depthTestEnable  = false,
            .depthWriteEnable = false,
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{
                .clear = true,
            }},
            .depthTestEnable = pipelineConfig.depthTestEnable,
            .discardDepthStencilAfterRender = true,
        };

    };
}