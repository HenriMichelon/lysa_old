/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.forward_renderer;

import vireo;
import lysa.global;

namespace lysa {
    ForwardRenderer::ForwardRenderer(
        const WindowConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const std::wstring& name) :
        MeshesRenderer{surfaceConfig, vireo, name},
        forwardColorPass{surfaceConfig, vireo, samplers} {
        framesData.resize(surfaceConfig.framesInFlight);
    }

    void ForwardRenderer::update(uint32 frameIndex) {
        forwardColorPass.update(frameIndex);
        std::ranges::for_each(postProcessingPasses, [&](const auto& postProcessingPass) {
            postProcessingPass->update(frameIndex);
        });
    }

    std::vector<std::shared_ptr<const vireo::CommandList>> ForwardRenderer::render(
        const uint32 frameIndex,
        Scene& scene) {
        const auto& frame = framesData[frameIndex];
        const auto& frameMeshes = MeshesRenderer::framesData[frameIndex];

        frameMeshes.commandAllocator->reset();
        auto commandList = frameMeshes.commandList;
        commandList->begin();
        commandList->barrier(frame.colorAttachment, vireo::ResourceState::UNDEFINED,vireo::ResourceState::RENDER_TARGET_COLOR);
        forwardColorPass.render(frameIndex, scene, frame.colorAttachment, commandList, false);

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

    void ForwardRenderer::resize(const vireo::Extent& extent) {
        Renderer::resize(extent);
        for (auto& frame : framesData) {
            frame.colorAttachment = vireo->createRenderTarget(
                surfaceConfig.renderingFormat,
                extent.width, extent.height,
                vireo::RenderTargetType::COLOR,
                {surfaceConfig.clearColor.r, surfaceConfig.clearColor.g, surfaceConfig.clearColor.b, 1.0f},
                surfaceConfig.msaa,
                name);
        }
        forwardColorPass.resize(extent);
        std::ranges::for_each(postProcessingPasses, [&extent](const auto& postProcessingPass) {
            postProcessingPass->resize(extent);
        });
    }


    std::shared_ptr<vireo::Image> ForwardRenderer::getColorAttachment(const uint32 frameIndex) const {
        if (postProcessingPasses.empty()) {
            return framesData[frameIndex].colorAttachment->getImage();
        }
        return  postProcessingPasses.back()->getColorAttachment(frameIndex);
    }
}