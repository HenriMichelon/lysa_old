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
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderpass;

export namespace lysa {
    class ForwardColor : public Renderpass {
    public:
        ForwardColor(const RenderingConfiguration& config);

        void updatePipelines(const std::unordered_map<uint32, std::shared_ptr<Material>>& materials);

        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex);

    private:
        const std::wstring DEFAULT_VERTEX_SHADER{L"default.vert"};
        const std::wstring DEFAULT_FRAGMENT_SHADER{L"forward.frag"};

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc   = { { .blendEnable = true } },
            .depthTestEnable  = true,
            .depthWriteEnable = true,
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{ }},
            .depthTestEnable = pipelineConfig.depthTestEnable,
            .discardDepthStencilAfterRender = true,
        };

        std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>> pipelines;

    };
}