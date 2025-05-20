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
import lysa.samplers;
import lysa.scene;
import lysa.renderers.renderpass;

export namespace lysa {
    class ForwardColor : public Renderpass {
    public:
        ForwardColor(
            const RenderingConfiguration& config,
            const Samplers& samplers);

        void render(
            uint32 frameIndex,
            Scene& scene,
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