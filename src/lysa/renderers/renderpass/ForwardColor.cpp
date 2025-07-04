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
        pipelineConfig.colorRenderFormats.push_back(config.colorRenderingFormat); // Color
        pipelineConfig.colorRenderFormats.push_back(config.colorRenderingFormat); // Brightness
        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.resources = Application::getVireo().createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::pipelineDescriptorLayout,
            Scene::sceneDescriptorLayoutOptional1},
            Scene::instanceIndexConstantDesc, name);
        pipelineConfig.vertexInputLayout = Application::getVireo().createVertexLayout(sizeof(VertexData), VertexData::vertexAttributes);

        renderingConfig.colorRenderTargets[0].clearValue = {
            config.clearColor.r,
            config.clearColor.g,
            config.clearColor.b,
            1.0f};
        renderingConfig.clearDepthStencil = false;

        framesData.resize(config.framesInFlight);
    }

    void ForwardColor::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        for (const auto& [pipelineId, materials] : pipelineIds) {
            if (!pipelines.contains(pipelineId)) {
                const auto& material = materials.at(0);
                std::wstring vertShaderName = DEFAULT_VERTEX_SHADER;
                std::wstring fragShaderName = DEFAULT_FRAGMENT_SHADER;
                pipelineConfig.cullMode = material->getCullMode();
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
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];
        if (buffersResized) {
            commandList.barrier(
                frame.brightnessBuffer,
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::SHADER_READ);
            buffersResized--;
        }

        renderingConfig.colorRenderTargets[0].clear = clearAttachment;
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        renderingConfig.colorRenderTargets[1].renderTarget = frame.brightnessBuffer;
        renderingConfig.depthStencilRenderTarget = depthAttachment;

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
            colorAttachment,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.barrier(
            frame.brightnessBuffer,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(renderingConfig);
        scene.drawOpaquesModels(
            commandList,
            pipelines);
        scene.drawTransparentModels(
            commandList,
            pipelines);
        commandList.endRendering();
        commandList.barrier(
            frame.brightnessBuffer,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::UNDEFINED);
        commandList.barrier(
            colorAttachment,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::UNDEFINED);
        commandList.barrier(
            depthAttachment,
            depthStage,
            vireo::ResourceState::UNDEFINED);
    }

    void ForwardColor::resize(const vireo::Extent& extent) {
        const auto& vireo = Application::getVireo();
        for (auto& frame : framesData) {
            frame.brightnessBuffer = vireo.createRenderTarget(
                pipelineConfig.colorRenderFormats[1],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[1].clearValue,
                1,
                vireo::MSAA::NONE,
                L"Brightness");
        }
        buffersResized = config.framesInFlight;
    }
}