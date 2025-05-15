/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.post_processing;

import lysa.global;

namespace lysa {
    PostProcessing::PostProcessing(
        const WindowConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const Samplers& samplers,
        const std::wstring& fragShaderName,
        void* data,
        const uint32 dataSize,
        const std::wstring& name):
        Renderpass{surfaceConfig, vireo, samplers, name},
        fragShaderName{fragShaderName},
        data{data},
        descriptorLayout{vireo->createDescriptorLayout(name)} {
        descriptorLayout->add(BINDING_PARAMS, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_INPUT, vireo::DescriptorType::SAMPLED_IMAGE);
        if (data) {
            descriptorLayout->add(BINDING_DATA, vireo::DescriptorType::UNIFORM);
            dataUniform = vireo->createBuffer(vireo::BufferType::UNIFORM, dataSize, 1, name + L" Data");
            dataUniform->map();
            dataUniform->write(data, dataSize);
            dataUniform->unmap();
        }
        descriptorLayout->build();

        pipelineConfig.colorRenderFormats.push_back(surfaceConfig.renderingFormat);
        pipelineConfig.resources = vireo->createPipelineResources({
            descriptorLayout,
            samplers.getDescriptorLayout()},
            {},
            name);
        pipelineConfig.vertexShader = vireo->createShaderModule("shaders/quad.vert");
        pipelineConfig.fragmentShader = vireo->createShaderModule("shaders/" + std::to_string(fragShaderName) + ".frag");
        pipeline = vireo->createGraphicPipeline(pipelineConfig, name);

        framesData.resize(surfaceConfig.framesInFlight);
        for (auto& frame : framesData) {
            frame.paramsUniform = vireo->createBuffer(vireo::BufferType::UNIFORM, sizeof(PostProcessingParams), 1, name + L" Params");
            frame.paramsUniform->map();
            frame.descriptorSet = vireo->createDescriptorSet(descriptorLayout, name);
            frame.descriptorSet->update(BINDING_PARAMS, frame.paramsUniform);
            if (data) {
                frame.descriptorSet->update(BINDING_DATA, dataUniform);
            }
        }
    }

    void PostProcessing::update(const uint32 frameIndex) {
        auto& frame = framesData[frameIndex];
        frame.params.time = getCurrentTimeMilliseconds();
        frame.paramsUniform->write(&frame.params, sizeof(frame.params));
    }

    void PostProcessing::render(
        const uint32 frameIndex,
        Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::CommandList>& commandList,
        const bool recordLastBarrier) {
        auto& frame = framesData[frameIndex];

        frame.descriptorSet->update(BINDING_INPUT, colorAttachment->getImage());
        renderingConfig.colorRenderTargets[0].renderTarget = frame.colorAttachment;
        commandList->barrier(
            frame.colorAttachment,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList->setDescriptors({frame.descriptorSet, samplers.getDescriptorSet()});
        commandList->beginRendering(renderingConfig);
        commandList->setViewport(scene.getViewport());
        commandList->setScissors(scene.getScissors());
        commandList->bindPipeline(pipeline);
        commandList->bindDescriptors(pipeline, {frame.descriptorSet, samplers.getDescriptorSet()});
        commandList->draw(3);
        commandList->endRendering();
        if (recordLastBarrier) {
            commandList->barrier(
                frame.colorAttachment,
                vireo::ResourceState::RENDER_TARGET_COLOR,
                vireo::ResourceState::UNDEFINED);
        }
    }

    void PostProcessing::resize(const vireo::Extent& extent) {
        for (auto& frame : framesData) {
            frame.colorAttachment = vireo->createRenderTarget(
                surfaceConfig.renderingFormat,
                extent.width, extent.height,
                vireo::RenderTargetType::COLOR,
    {},
                surfaceConfig.msaa,
                name);
            frame.params.imageSize.x = extent.width;
            frame.params.imageSize.y = extent.height;
        }
    }

}