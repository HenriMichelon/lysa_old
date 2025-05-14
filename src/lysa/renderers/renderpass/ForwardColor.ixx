/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.forward_color;

import std;
import vireo;
import lysa.surface_config;
import lysa.renderers.renderpass;
import lysa.renderers.samplers;

export namespace lysa {
    class ForwardColor : public Renderpass {
    public:
        ForwardColor(
            const SurfaceConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const Samplers& samplers);

        void render(
            uint32_t frameIndex,
            const vireo::Extent& extent,
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