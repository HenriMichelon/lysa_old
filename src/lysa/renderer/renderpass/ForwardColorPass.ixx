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

export namespace lysa {
    class ForwardColorPass : public Renderpass {
    public:
        ForwardColorPass(const SurfaceConfig& surfaceConfig);

        void render(
            uint32_t frameIndex,
            const vireo::Extent& extent,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::CommandList>& commandList) override;

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