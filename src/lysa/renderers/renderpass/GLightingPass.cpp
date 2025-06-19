/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.glighting_pass;

import lysa.application;
import lysa.resources;
import lysa.virtual_fs;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {

    GLightingPass::GLightingPass(
        const RenderingConfiguration& config,
        const GBufferPass& gBufferPass):
        Renderpass{config, L"GLighting"},
        gBufferPass{gBufferPass} {
        const auto& vireo = Application::getVireo();

        descriptorLayout = vireo.createDescriptorLayout();
        descriptorLayout->add(BINDING_POSITION_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->add(BINDING_NORMAL_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->add(BINDING_ALBEDO_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        // descriptorLayout->add(BINDING_MATERIAL_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->build();

        pipelineConfig.colorRenderFormats.push_back(config.renderingFormat);
        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.backStencilOpState = pipelineConfig.frontStencilOpState;
        pipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            descriptorLayout},
            {}, name);

        framesData.resize(config.framesInFlight);
        for (auto& frame : framesData) {
            frame.descriptorSet = vireo.createDescriptorSet(descriptorLayout);
        }

        renderingConfig.colorRenderTargets[0].clearValue = {
            config.clearColor.r,
            config.clearColor.g,
            config.clearColor.b,
            1.0f};
    }

    void GLightingPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!pipelines.contains(pipelineId)) {
                const auto& material = materials.at(0);
                std::wstring fragShaderName = DEFAULT_FRAGMENT_SHADER;
                if (material->getType() == Material::SHADER) {
                    const auto& shaderMaterial = std::dynamic_pointer_cast<const ShaderMaterial>(material);
                    if (!shaderMaterial->getFragFileName().empty()) {
                        fragShaderName = shaderMaterial->getFragFileName();
                    }
                }
                pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
                pipelineConfig.fragmentShader = loadShader(fragShaderName);
                pipelines[pipelineId] = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
            }
        }
    }

    void GLightingPass::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool clearAttachment,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];

        frame.descriptorSet->update(BINDING_POSITION_BUFFER, gBufferPass.getPositionBuffer(frameIndex)->getImage());
        frame.descriptorSet->update(BINDING_NORMAL_BUFFER, gBufferPass.getNormalBuffer(frameIndex)->getImage());
        frame.descriptorSet->update(BINDING_ALBEDO_BUFFER, gBufferPass.getAlbedoBuffer(frameIndex)->getImage());
        // frame.descriptorSet->update(BINDING_MATERIAL_BUFFER, gBufferPass.getMaterialBuffer(frameIndex)->getImage());

        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        renderingConfig.colorRenderTargets[0].clear = clearAttachment;
        renderingConfig.depthStencilRenderTarget = depthAttachment;

        for (const auto& pipeline : std::views::values(pipelines)) {
            commandList.bindPipeline(pipeline);
            commandList.bindDescriptors({
                 Application::getResources().getDescriptorSet(),
                 Application::getResources().getSamplers().getDescriptorSet(),
                 scene.getDescriptorSet(),
                 frame.descriptorSet
            });
            commandList.beginRendering(renderingConfig);
            commandList.setStencilReference(1);
            commandList.draw(3);
            commandList.endRendering();
        }
    }

}