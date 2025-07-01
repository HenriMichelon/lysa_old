/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.lighting_pass;

import lysa.application;
import lysa.resources;
import lysa.virtual_fs;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {

    LightingPass::LightingPass(
        const RenderingConfiguration& config,
        const GBufferPass& gBufferPass):
        Renderpass{config, L"Deferred Lighting"},
        gBufferPass{gBufferPass} {
        const auto& vireo = Application::getVireo();

        descriptorLayout = vireo.createDescriptorLayout();
        descriptorLayout->add(BINDING_POSITION_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->add(BINDING_NORMAL_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->add(BINDING_ALBEDO_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->add(BINDING_EMISSIVE_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->build();

        pipelineConfig.colorRenderFormats.push_back(config.renderingFormat);
        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.backStencilOpState = pipelineConfig.frontStencilOpState;
        pipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            descriptorLayout,
            Scene::sceneDescriptorLayoutOptional1},
            {}, name);
        pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
        pipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER);
        pipeline = vireo.createGraphicPipeline(pipelineConfig, name);

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

    void LightingPass::render(
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
        frame.descriptorSet->update(BINDING_EMISSIVE_BUFFER, gBufferPass.getEmissiveBuffer(frameIndex)->getImage());

        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        renderingConfig.colorRenderTargets[0].clear = clearAttachment;
        renderingConfig.depthStencilRenderTarget = depthAttachment;

        commandList.barrier(
             depthAttachment,
             vireo::ResourceState::UNDEFINED,
             vireo::ResourceState::RENDER_TARGET_DEPTH_STENCIL);
        commandList.barrier(
             colorAttachment,
             vireo::ResourceState::UNDEFINED,
             vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors({
             Application::getResources().getDescriptorSet(),
             Application::getResources().getSamplers().getDescriptorSet(),
             scene.getDescriptorSet(),
             frame.descriptorSet,
             scene.getDescriptorSetOptional1()
        });
        commandList.beginRendering(renderingConfig);
        commandList.setStencilReference(1);
        commandList.draw(3);
        commandList.endRendering();
        commandList.barrier(
           depthAttachment,
           vireo::ResourceState::RENDER_TARGET_DEPTH_STENCIL,
           vireo::ResourceState::UNDEFINED);
        commandList.barrier(
           colorAttachment,
           vireo::ResourceState::RENDER_TARGET_COLOR,
           vireo::ResourceState::UNDEFINED);
    }

}