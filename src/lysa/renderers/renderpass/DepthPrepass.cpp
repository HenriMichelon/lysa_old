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
        const RenderingConfiguration& config):
        Renderpass{config, L"Depth pre-pass"} {
        const auto& vireo = Application::getVireo();
        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::drawCommandDescriptorLayout},
            {}, name);
    }

    void DepthPrepass::updatePipelines(const std::unordered_map<uint32, std::shared_ptr<Material>>& materials) {
        for (const auto& [pipelineId, material] : materials) {
            if (!pipelines.contains(pipelineId)) {
                std::wstring vertShaderName = L"depth_prepass.vert";
                pipelineConfig.cullMode = material->getCullMode();
                auto tempBuffer = std::vector<char>{};
                const auto& ext = Application::getVireo().getShaderFileExtension();
                VirtualFS::loadBinaryData(L"app://shaders/" + vertShaderName + ext, tempBuffer);
                pipelineConfig.vertexShader = Application::getVireo().createShaderModule(tempBuffer);
                pipelines[pipelineId] = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
            }
        }
    }

    void DepthPrepass::render(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment) {
        renderingConfig.depthStencilRenderTarget = depthAttachment;
        commandList.beginRendering(renderingConfig);
        scene.drawOpaquesModels(
          commandList,
          pipelines);
        commandList.endRendering();
    }
}