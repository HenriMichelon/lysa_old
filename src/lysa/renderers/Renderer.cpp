/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderer;

import lysa.application;

namespace lysa {
    Renderer::Renderer(
        const RenderingConfiguration& config,
        const std::wstring& name) :
        config{config},
        name{name} {
        framesData.resize(config.framesInFlight);
        for (auto& frame : framesData) {
            frame.commandAllocator = Application::getVireo().createCommandAllocator(vireo::CommandType::GRAPHIC);
            frame.commandList = frame.commandAllocator->createCommandList();
        }
    }

    void Renderer::update(const uint32 frameIndex) {
        for (const auto& postProcessingPass : postProcessingPasses) {
            postProcessingPass->update(frameIndex);
        }
    }

    std::vector<std::shared_ptr<const vireo::CommandList>> Renderer::render(
        const uint32 frameIndex,
        Scene& scene) {
        const auto& frame = framesData[frameIndex];

        frame.commandAllocator->reset();
        auto commandList = frame.commandList;
        commandList->begin();

        scene.update(*commandList);
        update(frameIndex);

        commandList->barrier(frame.colorAttachment, vireo::ResourceState::UNDEFINED,vireo::ResourceState::RENDER_TARGET_COLOR);

        mainColorPass(frameIndex, scene, frame.colorAttachment, commandList);

        if (!postProcessingPasses.empty()) {
            commandList->barrier(
                frame.colorAttachment,
                vireo::ResourceState::RENDER_TARGET_COLOR,
                vireo::ResourceState::SHADER_READ);
            std::ranges::for_each(postProcessingPasses, [&](const auto& postProcessingPass) {
                postProcessingPass->render(
                    frameIndex,
                    scene,
                    frame.colorAttachment,
                    commandList,
                    postProcessingPass != postProcessingPasses.back());
            });
            commandList->barrier(
                frame.colorAttachment,
                vireo::ResourceState::SHADER_READ,
                vireo::ResourceState::UNDEFINED);
        }
        return {commandList};
    }

    std::shared_ptr<vireo::Image> Renderer::getColorAttachment(const uint32 frameIndex) const {
        if (postProcessingPasses.empty()) {
            return framesData[frameIndex].colorAttachment->getImage();
        }
        return  postProcessingPasses.back()->getColorAttachment(frameIndex);
    }

    void Renderer::resize(const vireo::Extent& extent) {
        currentExtent = extent;
        for (auto& frame : framesData) {
            frame.colorAttachment = Application::getVireo().createRenderTarget(
                config.renderingFormat,
                extent.width, extent.height,
                vireo::RenderTargetType::COLOR,
                {config.clearColor.r, config.clearColor.g, config.clearColor.b, 1.0f},
                config.msaa,
                name);
        }
        for (const auto& postProcessingPass : postProcessingPasses) {
            postProcessingPass->resize(extent);
        }
    }

    void Renderer::addPostprocessing(const std::wstring& fragShaderName, void* data, const uint32 dataSize) {
        const auto postProcessingPass = std::make_shared<PostProcessing>(
            config,
            samplers,
            fragShaderName,
            data,
            dataSize,
            fragShaderName);
        postProcessingPass->resize(currentExtent);
        postProcessingPasses.push_back(postProcessingPass);
    }

    void Renderer::removePostprocessing(const std::wstring& fragShaderName) {
        std::erase_if(postProcessingPasses, [&fragShaderName](const std::shared_ptr<PostProcessing>& item) {
            return item->getFragShaderName() == fragShaderName;
        });
    }

}