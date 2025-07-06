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
        const vireo::ImageFormat outputFormat,
        void* data,
        const uint32 dataSize,
        const std::wstring& name):
        Renderpass{config, name},
        fragShaderName{fragShaderName},
        data{data},
        descriptorLayout{Application::getVireo().createDescriptorLayout(name)} {
        textures.resize(TEXTURES_COUNT);
        for (int i = 0; i < TEXTURES_COUNT; i++) {
            textures[i] = Application::getResources().getBlankImage();
        }

        descriptorLayout->add(BINDING_PARAMS, vireo::DescriptorType::UNIFORM);
        if (data) {
            descriptorLayout->add(BINDING_DATA, vireo::DescriptorType::UNIFORM);
            dataUniform = Application::getVireo().createBuffer(vireo::BufferType::UNIFORM, dataSize, 1, name + L" Data");
            dataUniform->map();
        }
        descriptorLayout->add(BINDING_TEXTURES, vireo::DescriptorType::SAMPLED_IMAGE, TEXTURES_COUNT);
        descriptorLayout->build();

        const auto& vireo = Application::getVireo();
        pipelineConfig.colorRenderFormats.push_back(outputFormat); //useRenderingColorAttachmentFormat ? config.colorRenderingFormat : config.swapChainFormat);
        pipelineConfig.resources = vireo.createPipelineResources({
            descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout()},
            {},
            name);
        pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
        pipelineConfig.fragmentShader = loadShader(fragShaderName + L".frag");
        pipeline = vireo.createGraphicPipeline(pipelineConfig, name);

        framesData.resize(config.framesInFlight);
        for (auto& frame : framesData) {
            frame.paramsUniform = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(PostProcessingParams), 1, name + L" Params");
            frame.paramsUniform->map();
            frame.params.applyBloom = config.bloomEnabled ? 1u : 0u;
            frame.descriptorSet = vireo.createDescriptorSet(descriptorLayout, name);
            frame.descriptorSet->update(BINDING_PARAMS, frame.paramsUniform);
            if (data) {
                frame.descriptorSet->update(BINDING_DATA, dataUniform);
            }
        }
    }

    void PostProcessing::update(const uint32 frameIndex) {
        auto& frame = framesData[frameIndex];
        frame.params.time = 123.45; //getCurrentTimeMilliseconds();
        frame.paramsUniform->write(&frame.params, sizeof(frame.params));
        if (data) {
            dataUniform->write(data);
        }
    }

    void PostProcessing::render(
        const uint32 frameIndex,
        const vireo::Viewport& viewport,
        const vireo::Rect& scissor,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const std::shared_ptr<vireo::RenderTarget>& bloomColorAttachment,
        vireo::CommandList& commandList) {
        auto& frame = framesData[frameIndex];

        textures[INPUT_BUFFER] = colorAttachment->getImage();
        if (depthAttachment) { textures[DEPTH_BUFFER] = depthAttachment->getImage(); }
        if (bloomColorAttachment) { textures[BLOOM_BUFFER] = bloomColorAttachment->getImage(); }
        frame.descriptorSet->update(BINDING_TEXTURES, textures);

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
            Application::getResources().getSamplers().getDescriptorSet()});
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