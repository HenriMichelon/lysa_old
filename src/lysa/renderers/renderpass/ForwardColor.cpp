/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.forward_color;

import lysa.application;
import lysa.resources;
import lysa.virtual_fs;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {
    ForwardColor::ForwardColor(
        const RenderingConfiguration& config,
        const Samplers& samplers):
        Renderpass{config, samplers, L"Forward Color"} {
        pipelineConfig.colorRenderFormats.push_back(config.renderingFormat);
        pipelineConfig.resources = Application::getVireo().createPipelineResources({
            Resources::descriptorLayout,
            samplers.getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::drawCommandDescriptorLayout},
            {}, name);
        renderingConfig.colorRenderTargets[0].clearValue = {
            config.clearColor.r,
            config.clearColor.g,
            config.clearColor.b,
            1.0f};
    }

    void ForwardColor::updatePipelines(const std::unordered_map<uint32, std::shared_ptr<Material>>& materials) {
        for (const auto& [pipelineId, material] : materials) {
            if (!pipelines.contains(pipelineId)) {
                std::wstring vertShaderName = DEFAULT_VERTEX_SHADER;
                std::wstring fragShaderName = DEFAULT_FRAGMENT_SHADER;
                if (pipelineId != 0) {
                    const auto& shaderMaterial = dynamic_pointer_cast<const ShaderMaterial>(material);
                    if (!shaderMaterial->getVertFileName().empty()) {
                        vertShaderName = shaderMaterial->getVertFileName();
                    }
                    if (!shaderMaterial->getFragFileName().empty()) {
                        fragShaderName = shaderMaterial->getFragFileName();
                    }
                }
                pipelineConfig.colorBlendDesc[0].blendEnable = material->getTransparency() != Transparency::DISABLED;
                pipelineConfig.cullMode = material->getCullMode();
                auto tempBuffer = std::vector<char>{};
                const auto& ext = Application::getVireo().getShaderFileExtension();
                VirtualFS::loadBinaryData(L"app://shaders/" + vertShaderName + ext, tempBuffer);
                pipelineConfig.vertexShader = Application::getVireo().createShaderModule(tempBuffer);
                VirtualFS::loadBinaryData(L"app://shaders/" + fragShaderName + ext, tempBuffer);
                pipelineConfig.fragmentShader = Application::getVireo().createShaderModule(tempBuffer);
                pipelines[pipelineId] = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
            }
        }
    }

    void ForwardColor::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const bool clearAttachment,
        const uint32) {
        renderingConfig.colorRenderTargets[0].clear = clearAttachment;
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        commandList.beginRendering(renderingConfig);
        scene.drawOpaquesModels(
          commandList,
          pipelines,
          samplers);
        commandList.endRendering();
    }
}