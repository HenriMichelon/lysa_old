/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.gbuffer_pass;

import lysa.application;
import lysa.resources;
import lysa.virtual_fs;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {
    GBufferPass::GBufferPass(
        const RenderingConfiguration& config):
        Renderpass{config, L"GBuffer"} {

        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.backStencilOpState = pipelineConfig.frontStencilOpState;
        pipelineConfig.resources = Application::getVireo().createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::pipelineDescriptorLayout},
            Scene::instanceIndexConstantDesc, name);
        pipelineConfig.vertexInputLayout = Application::getVireo().createVertexLayout(sizeof(VertexData), VertexData::vertexAttributes);

        framesData.resize(config.framesInFlight);
    }

    void GBufferPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!pipelines.contains(pipelineId)) {
                const auto& material = materials.at(0);
                pipelineConfig.cullMode = material->getCullMode();
                pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
                pipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER);
                pipelines[pipelineId] = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
            }
        }
    }

    void GBufferPass::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>&,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];
        if (buffersResized) {
            commandList.barrier(
                {
                    frame.positionBuffer,
                    frame.normalBuffer,
                    frame.albedoBuffer,
                    frame.emissiveBuffer
                },
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::SHADER_READ);
            buffersResized--;
        }

        renderingConfig.colorRenderTargets[BUFFER_POSITION].renderTarget = frame.positionBuffer;
        renderingConfig.colorRenderTargets[BUFFER_NORMAL].renderTarget = frame.normalBuffer;
        renderingConfig.colorRenderTargets[BUFFER_ALBEDO].renderTarget = frame.albedoBuffer;
        renderingConfig.colorRenderTargets[BUFFER_EMISSIVE].renderTarget = frame.emissiveBuffer;
        renderingConfig.depthStencilRenderTarget = depthAttachment;

        auto renderTargets = std::views::transform(renderingConfig.colorRenderTargets, [](const auto& colorRenderTarget) {
            return colorRenderTarget.renderTarget;
        });
        commandList.barrier(
            depthAttachment,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_DEPTH_STENCIL);
        commandList.barrier(
           {renderTargets.begin(), renderTargets.end()},
           vireo::ResourceState::SHADER_READ,
           vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(renderingConfig);
        commandList.setStencilReference(1);
        scene.drawOpaquesModels(
          commandList,
          pipelines);
        scene.drawTransparentModels(
          commandList,
          pipelines);
        commandList.endRendering();
        commandList.barrier(
            {renderTargets.begin(), renderTargets.end()},
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);
        commandList.barrier(
            depthAttachment,
            vireo::ResourceState::RENDER_TARGET_DEPTH_STENCIL,
            vireo::ResourceState::UNDEFINED);
    }

    void GBufferPass::resize(const vireo::Extent& extent) {
        const auto& vireo = Application::getVireo();
        for (auto& frame : framesData) {
            frame.positionBuffer = vireo.createRenderTarget(
                pipelineConfig.colorRenderFormats[BUFFER_POSITION],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[BUFFER_POSITION].clearValue,
                vireo::MSAA::NONE,
                L"Position");
            frame.normalBuffer = vireo.createRenderTarget(
                pipelineConfig.colorRenderFormats[BUFFER_NORMAL],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[BUFFER_NORMAL].clearValue,
                vireo::MSAA::NONE,
                L"Normal");
            frame.albedoBuffer = vireo.createRenderTarget(
                pipelineConfig.colorRenderFormats[BUFFER_ALBEDO],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[BUFFER_ALBEDO].clearValue,
                vireo::MSAA::NONE,
                L"Albedo");
            frame.emissiveBuffer = vireo.createRenderTarget(
                pipelineConfig.colorRenderFormats[BUFFER_EMISSIVE],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[BUFFER_EMISSIVE].clearValue,
                vireo::MSAA::NONE,
                L"Emissive");
        }
        buffersResized = config.framesInFlight;
    }
}