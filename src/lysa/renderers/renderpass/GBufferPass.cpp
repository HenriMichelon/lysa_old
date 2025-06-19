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
                pipelineConfig.cullMode = material->getCullMode();
                auto tempBuffer = std::vector<char>{};
                const auto& ext = Application::getVireo().getShaderFileExtension();
                VirtualFS::loadBinaryData(L"app://" + Application::getConfiguration().shaderDir + L"/" + vertShaderName + ext, tempBuffer);
                pipelineConfig.vertexShader = Application::getVireo().createShaderModule(tempBuffer);
                VirtualFS::loadBinaryData(L"app://"+ Application::getConfiguration().shaderDir + L"/" + fragShaderName + ext, tempBuffer);
                pipelineConfig.fragmentShader = Application::getVireo().createShaderModule(tempBuffer);
                pipelines[pipelineId] = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
            }
        }
    }

    void GBufferPass::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool clearAttachment,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];
        if (buffersResized) {
            commandList.barrier(
                {
                    frame.positionBuffer,
                    frame.normalBuffer,
                    frame.albedoBuffer,
                    // frame.materialBuffer
                },
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::SHADER_READ);
            buffersResized--;
        }

        renderingConfig.colorRenderTargets[BUFFER_POSITION].renderTarget = frame.positionBuffer;
        renderingConfig.colorRenderTargets[BUFFER_NORMAL].renderTarget = frame.normalBuffer;
        renderingConfig.colorRenderTargets[BUFFER_ALBEDO].renderTarget = frame.albedoBuffer;
        // renderingConfig.colorRenderTargets[BUFFER_MATERIAL].renderTarget = frame.materialBuffer;
        renderingConfig.depthStencilRenderTarget = depthAttachment;

        auto renderTargets = std::views::transform(renderingConfig.colorRenderTargets, [](const auto& colorRenderTarget) {
            return colorRenderTarget.renderTarget;
        });
        commandList.barrier(
           {renderTargets.begin(), renderTargets.end()},
           vireo::ResourceState::SHADER_READ,
           vireo::ResourceState::RENDER_TARGET_COLOR);

        commandList.beginRendering(renderingConfig);
        commandList.setStencilReference(1);
        scene.drawOpaquesModels(
          commandList,
          pipelines);
        commandList.endRendering();

        commandList.barrier(
            {renderTargets.begin(), renderTargets.end()},
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);
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
            // frame.materialBuffer = vireo->createRenderTarget(
            //     pipelineConfig.colorRenderFormats[BUFFER_MATERIAL],
            //     extent.width,extent.height,
            //     vireo::RenderTargetType::COLOR,
            //     renderingConfig.colorRenderTargets[BUFFER_MATERIAL].clearValue);
        }
        buffersResized = config.framesInFlight;
    }
}