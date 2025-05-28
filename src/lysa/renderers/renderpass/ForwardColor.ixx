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
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            bool clearAttachment,
            uint32 frameIndex);

    private:
        const std::string DEFAULT_VERTEX_SHADER = "default.vert";
        const std::string DEFAULT_FRAGMENT_SHADER = "forward.frag";

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc   = { { .blendEnable = true } },
            .depthTestEnable  = false,
            .depthWriteEnable = false,
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{ }},
            .depthTestEnable = pipelineConfig.depthTestEnable,
            .discardDepthStencilAfterRender = true,
        };

    };
}