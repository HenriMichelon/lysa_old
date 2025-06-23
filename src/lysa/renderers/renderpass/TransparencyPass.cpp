/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.transparency_pass;

import lysa.application;
import lysa.resources;
import lysa.virtual_fs;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {

    TransparencyPass::TransparencyPass(
        const RenderingConfiguration& config):
        Renderpass{config, L"Deferred Transparency"} {
        const auto& vireo = Application::getVireo();

        oitPipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        oitPipelineConfig.backStencilOpState = oitPipelineConfig.frontStencilOpState;
        oitPipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::pipelineDescriptorLayout},
            Scene::instanceIndexConstantDesc, name);
        oitPipelineConfig.vertexShader = loadShader(VERTEX_SHADER_OIT);
        oitPipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER_OIT);
        oitPipelineConfig.vertexInputLayout = Application::getVireo().createVertexLayout(sizeof(VertexData), VertexData::vertexAttributes);

        compositeDescriptorLayout = vireo.createDescriptorLayout();
        compositeDescriptorLayout->add(BINDING_ACCUM_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        compositeDescriptorLayout->add(BINDING_REVEALAGE_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        compositeDescriptorLayout->build();

        compositePipelineConfig.colorRenderFormats.push_back(config.renderingFormat);
        compositePipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            compositeDescriptorLayout},
            {}, name);
        compositePipelineConfig.vertexShader = loadShader(VERTEX_SHADER_COMPOSITE);
        compositePipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER_COMPOSITE);
        compositePipeline = vireo.createGraphicPipeline(compositePipelineConfig, L"Transparency OIT Composite");

        framesData.resize(config.framesInFlight);
        for (auto& frame : framesData) {
            frame.compositeDescriptorSet = vireo.createDescriptorSet(compositeDescriptorLayout, name);
        }
    }

    void TransparencyPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!oitPipelines.contains(pipelineId)) {
                const auto& material = materials.at(0);
                std::wstring fragShaderName = FRAGMENT_SHADER_OIT;
                oitPipelineConfig.cullMode = material->getCullMode();
                oitPipelineConfig.vertexShader = loadShader(VERTEX_SHADER_OIT);
                oitPipelineConfig.fragmentShader = loadShader(fragShaderName);
                oitPipelines[pipelineId] = Application::getVireo().createGraphicPipeline(oitPipelineConfig, L"Transparency OIT");
            }
        }
    }

    void TransparencyPass::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];
        if (buffersResized) {
            commandList.barrier(
                {
                    frame.accumBuffer,
                    frame.revealageBuffer,
                },
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::SHADER_READ);
            buffersResized--;
        }

        oitRenderingConfig.colorRenderTargets[BINDING_ACCUM_BUFFER].renderTarget = frame.accumBuffer;
        oitRenderingConfig.colorRenderTargets[BINDING_REVEALAGE_BUFFER].renderTarget = frame.revealageBuffer;
        oitRenderingConfig.depthStencilRenderTarget = depthAttachment;

        const auto depthStage =
         config.depthStencilFormat == vireo::ImageFormat::D32_SFLOAT_S8_UINT ||
         config.depthStencilFormat == vireo::ImageFormat::D24_UNORM_S8_UINT   ?
         vireo::ResourceState::RENDER_TARGET_DEPTH_STENCIL :
         vireo::ResourceState::RENDER_TARGET_DEPTH;

        commandList.barrier(
                depthAttachment,
                vireo::ResourceState::UNDEFINED,
                depthStage);
        commandList.barrier(
            {frame.accumBuffer, frame.revealageBuffer},
            vireo::ResourceState::SHADER_READ,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(oitRenderingConfig);
        scene.drawTransparentModels(commandList, oitPipelines);
        commandList.endRendering();
        commandList.barrier(
            {frame.accumBuffer, frame.revealageBuffer},
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);
        commandList.barrier(
            depthAttachment,
            depthStage,
            vireo::ResourceState::UNDEFINED);

        frame.compositeDescriptorSet->update(BINDING_ACCUM_BUFFER, frame.accumBuffer->getImage());
        frame.compositeDescriptorSet->update(BINDING_REVEALAGE_BUFFER, frame.revealageBuffer->getImage());
        compositeRenderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;

        commandList.barrier(
             colorAttachment,
             vireo::ResourceState::UNDEFINED,
             vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(compositeRenderingConfig);
        commandList.bindPipeline(compositePipeline);
        commandList.bindDescriptors({
            Application::getResources().getDescriptorSet(),
            Application::getResources().getSamplers().getDescriptorSet(),
            scene.getDescriptorSet(),
            frame.compositeDescriptorSet
       });
        commandList.draw(3);
        commandList.endRendering();
        commandList.barrier(
            colorAttachment,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::UNDEFINED);
    }

    void TransparencyPass::resize(const vireo::Extent& extent) {
        const auto& vireo = Application::getVireo();
        for (auto& frame : framesData) {
            frame.accumBuffer = vireo.createRenderTarget(
                oitPipelineConfig.colorRenderFormats[BINDING_ACCUM_BUFFER],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                oitRenderingConfig.colorRenderTargets[BINDING_ACCUM_BUFFER].clearValue);
            frame.revealageBuffer = vireo.createRenderTarget(
                oitPipelineConfig.colorRenderFormats[BINDING_REVEALAGE_BUFFER],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                oitRenderingConfig.colorRenderTargets[BINDING_REVEALAGE_BUFFER].clearValue);
        }
        buffersResized = config.framesInFlight;
    }

}