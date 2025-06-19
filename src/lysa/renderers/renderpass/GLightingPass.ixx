/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.glighting_pass;

import std;
import vireo;
import lysa.configuration;
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderpass;
import lysa.renderers.renderpass.gbuffer_pass;

export namespace lysa {

    class GLightingPass : public Renderpass {
    public:
        GLightingPass(const RenderingConfiguration& config, const GBufferPass& gBufferPass);

        void updatePipelines(
            const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds);

        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex);

    private:
        const std::wstring DEFAULT_VERTEX_SHADER{L"quad.vert"};
        const std::wstring DEFAULT_FRAGMENT_SHADER{L"glighting.frag"};

        static constexpr vireo::DescriptorIndex BINDING_POSITION_BUFFER{0};
        static constexpr vireo::DescriptorIndex BINDING_NORMAL_BUFFER{1};
        static constexpr vireo::DescriptorIndex BINDING_ALBEDO_BUFFER{2};
        // static constexpr vireo::DescriptorIndex BINDING_MATERIAL_BUFFER{3};

        struct FrameData {
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc = {{}},
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{
                .clear = false
            }},
            .depthTestEnable    = pipelineConfig.depthTestEnable,
            .stencilTestEnable  = pipelineConfig.stencilTestEnable,
        };

        const GBufferPass& gBufferPass;
        std::vector<FrameData> framesData;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
        std::unordered_map<pipeline_id, std::shared_ptr<vireo::GraphicPipeline>> pipelines;
    };
}