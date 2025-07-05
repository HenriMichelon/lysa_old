/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.post_processing;

import lysa.application;
import lysa.global;
import lysa.virtual_fs;

namespace lysa {
    PostProcessing::PostProcessing(
        const RenderingConfiguration& config,
        const std::wstring& fragShaderName,
        const bool useRenderingColorAttachmentFormat,
        void* data,
        const uint32 dataSize,
        const std::wstring& name):
        Renderpass{config, name},
        fragShaderName{fragShaderName},
        data{data},
        descriptorLayout{Application::getVireo().createDescriptorLayout(name)},
        descriptorLayoutBuffers{Application::getVireo().createDescriptorLayout(name)}{

        descriptorLayout->add(BINDING_PARAMS, vireo::DescriptorType::UNIFORM);
        if (data) {
            descriptorLayout->add(BINDING_DATA, vireo::DescriptorType::UNIFORM);
            dataUniform = Application::getVireo().createBuffer(vireo::BufferType::UNIFORM, dataSize, 1, name + L" Data");
            dataUniform->map();
            dataUniform->write(data, dataSize);
            dataUniform->unmap();
        }
        descriptorLayout->build();

        descriptorLayoutBuffers->add(BINDING_INPUT, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayoutBuffers->add(BINDING_DEPTH_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayoutBuffers->build();

        const auto& vireo = Application::getVireo();
        pipelineConfig.colorRenderFormats.push_back(useRenderingColorAttachmentFormat ? config.colorRenderingFormat : config.swapChainFormat);
        pipelineConfig.resources = vireo.createPipelineResources({
            descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            descriptorLayoutBuffers},
            {},
            name);
        pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
        pipelineConfig.fragmentShader = loadShader(fragShaderName + L".frag");
        pipeline = vireo.createGraphicPipeline(pipelineConfig, name);

        framesData.resize(config.framesInFlight);
        for (auto& frame : framesData) {
            frame.paramsUniform = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(PostProcessingParams), 1, name + L" Params");
            frame.paramsUniform->map();
            frame.descriptorSet = vireo.createDescriptorSet(descriptorLayout, name);
            frame.descriptorSet->update(BINDING_PARAMS, frame.paramsUniform);
            if (data) {
                frame.descriptorSet->update(BINDING_DATA, dataUniform);
            }
            frame.descriptorSetBuffers = vireo.createDescriptorSet(descriptorLayoutBuffers, name);
        }
    }

    void PostProcessing::update(const uint32 frameIndex) {
        auto& frame = framesData[frameIndex];
        frame.params.time = getCurrentTimeMilliseconds();
        frame.paramsUniform->write(&frame.params, sizeof(frame.params));
    }

    void PostProcessing::render(
        const uint32 frameIndex,
        const vireo::Viewport& viewport,
        const vireo::Rect& scissor,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        vireo::CommandList& commandList) {
        auto& frame = framesData[frameIndex];

        frame.descriptorSetBuffers->update(BINDING_INPUT, colorAttachment->getImage());
        frame.descriptorSetBuffers->update(BINDING_DEPTH_BUFFER, depthAttachment->getImage());
        renderingConfig.colorRenderTargets[0].renderTarget = frame.colorAttachment;
        commandList.barrier(
            frame.colorAttachment,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.beginRendering(renderingConfig);
        commandList.setViewport(viewport);
        commandList.setScissors(scissor);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors({
            frame.descriptorSet,
            Application::getResources().getSamplers().getDescriptorSet(),
            frame.descriptorSetBuffers});
        commandList.draw(3);
        commandList.endRendering();
        commandList.barrier(
            frame.colorAttachment,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);
    }

    void PostProcessing::resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) {
        for (auto& frame : framesData) {
            frame.colorAttachment = Application::getVireo().createRenderTarget(
                pipelineConfig.colorRenderFormats[0],
                extent.width, extent.height,
                vireo::RenderTargetType::COLOR,
    {},
                1,
                config.msaa,
                name);
            frame.params.imageSize.x = extent.width;
            frame.params.imageSize.y = extent.height;
        }
    }

}