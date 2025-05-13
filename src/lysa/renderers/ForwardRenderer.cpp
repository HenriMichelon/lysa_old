/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.forward_renderer;

import vireo;
import lysa.tools;

namespace lysa {
    ForwardRenderer::ForwardRenderer(
        const SurfaceConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const std::wstring& name) :
        MeshesRenderer{surfaceConfig, vireo, name},
        forwardColorPass{surfaceConfig, vireo, samplers} {
        framesData.resize(surfaceConfig.framesInFlight);
    }

    void ForwardRenderer::update(uint32_t frameIndex) {
        forwardColorPass.update(frameIndex);
        std::ranges::for_each(postProcessingPasses, [&](const auto& postProcessingPass) {
            postProcessingPass->update(frameIndex);
        });
    }

    void ForwardRenderer::render(
        const uint32_t frameIndex,
        const vireo::Extent& extent,
        const std::shared_ptr<vireo::Semaphore>& renderingFinishedSemaphore) {
        const auto& frame = framesData[frameIndex];
        const auto& frameMeshes = MeshesRenderer::framesData[frameIndex];

        frameMeshes.commandAllocator->reset();
        const auto commandList = frameMeshes.commandList;
        commandList->begin();
        commandList->barrier(frame.colorAttachment, vireo::ResourceState::UNDEFINED,vireo::ResourceState::RENDER_TARGET_COLOR);

        forwardColorPass.render(frameIndex, extent, frame.colorAttachment, commandList);

        if (postProcessingPasses.empty()) {
            commandList->barrier(
                frame.colorAttachment,
                vireo::ResourceState::RENDER_TARGET_COLOR,
                vireo::ResourceState::UNDEFINED);
        } else {
            commandList->barrier(
                frame.colorAttachment,
                vireo::ResourceState::RENDER_TARGET_COLOR,
                vireo::ResourceState::SHADER_READ);
            std::ranges::for_each(postProcessingPasses, [&](const auto& postProcessingPass) {
                postProcessingPass->render(frameIndex, extent, frame.colorAttachment, commandList);
            });
            commandList->barrier(
                frame.colorAttachment,
                vireo::ResourceState::SHADER_READ,
                vireo::ResourceState::UNDEFINED);
        }

        commandList->end();
        submitQueue->submit(vireo::WaitStage::COLOR_OUTPUT, renderingFinishedSemaphore, {commandList});
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


    std::shared_ptr<vireo::Image> ForwardRenderer::getColorAttachment(const uint32_t frameIndex) const {
        if (postProcessingPasses.empty()) {
            return framesData[frameIndex].colorAttachment->getImage();
        }
        return  postProcessingPasses.back()->getColorAttachment(frameIndex);
    }
}