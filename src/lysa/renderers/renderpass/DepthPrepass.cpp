/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.depth_prepass;

import lysa.application;
import lysa.resources;
import lysa.virtual_fs;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {
    DepthPrepass::DepthPrepass(
        const RenderingConfiguration& config,
        bool withStencil):
        Renderpass{config, L"Depth pre-pass"} {
        const auto& vireo = Application::getVireo();
        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.stencilTestEnable = withStencil;
        pipelineConfig.backStencilOpState = pipelineConfig.frontStencilOpState;
        pipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::pipelineDescriptorLayout},
            Scene::instanceIndexConstantDesc, name);
        renderingConfig.stencilTestEnable = pipelineConfig.stencilTestEnable;
    }

    void DepthPrepass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        const auto& vireo = Application::getVireo();
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!pipelines.contains(pipelineId)) {
                const auto& material = materials.at(0);
                pipelineConfig.cullMode = material->getCullMode();
                pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
                pipelineConfig.vertexInputLayout = vireo.createVertexLayout(sizeof(VertexData), VertexData::vertexAttributes);
                pipelines[pipelineId] = vireo.createGraphicPipeline(pipelineConfig, name);
            }
        }
    }

    void DepthPrepass::render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment) {
        renderingConfig.depthStencilRenderTarget = depthAttachment;
        commandList.beginRendering(renderingConfig);
        if (pipelineConfig.stencilTestEnable) {
            commandList.setStencilReference(1);
        }
        scene.drawOpaquesModels(
          commandList,
          pipelines);
        commandList.endRendering();
    }
}