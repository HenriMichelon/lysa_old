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
        const RenderingConfiguration& config):
        Renderpass{config, L"Forward Color"} {
        pipelineConfig.colorRenderFormats.push_back(config.renderingFormat);
        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.depthWriteEnable = true; //!config.forwardDepthPrepass;
        pipelineConfig.resources = Application::getVireo().createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::pipelineDescriptorLayout},
            Scene::instanceIndexConstantDesc, name);
        pipelineConfig.vertexInputLayout = Application::getVireo().createVertexLayout(sizeof(VertexData), VertexData::vertexAttributes);

        renderingConfig.colorRenderTargets[0].clearValue = {
            config.clearColor.r,
            config.clearColor.g,
            config.clearColor.b,
            1.0f};
        renderingConfig.clearDepthStencil = false; //!config.forwardDepthPrepass;
    }

    void ForwardColor::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!pipelines.contains(pipelineId)) {
                const auto& material = materials.at(0);
                std::wstring vertShaderName = DEFAULT_VERTEX_SHADER;
                std::wstring fragShaderName = DEFAULT_FRAGMENT_SHADER;
                if (material->getType() == Material::SHADER) {
                    const auto& shaderMaterial = std::dynamic_pointer_cast<const ShaderMaterial>(material);
                    if (!shaderMaterial->getVertFileName().empty()) {
                        vertShaderName = shaderMaterial->getVertFileName();
                    }
                    if (!shaderMaterial->getFragFileName().empty()) {
                        fragShaderName = shaderMaterial->getFragFileName();
                    }
                }
                pipelineConfig.colorBlendDesc[0].blendEnable = material->getTransparency() != Transparency::DISABLED;
                pipelineConfig.cullMode = material->getCullMode();
                pipelineConfig.depthWriteEnable = true; //material->getTransparency() == Transparency::DISABLED;
                pipelineConfig.vertexShader = loadShader(vertShaderName);
                pipelineConfig.fragmentShader = loadShader(fragShaderName);
                pipelines[pipelineId] = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
            }
        }
    }

    void ForwardColor::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool clearAttachment,
        const uint32) {
        renderingConfig.colorRenderTargets[0].clear = clearAttachment;
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        renderingConfig.depthStencilRenderTarget = depthAttachment;
        renderingConfig.discardDepthStencilAfterRender = true;

        commandList.beginRendering(renderingConfig);
        scene.drawOpaquesModels(
          commandList,
          pipelines);
        scene.drawTransparentModels(
          commandList,
          pipelines);
        commandList.endRendering();
    }
}