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
        MeshesRenderer{surfaceConfig, vireo, name} {
        framesData.resize(surfaceConfig.framesInFlight);
        pipelineConfig.colorRenderFormats.push_back(surfaceConfig.renderingFormat);
        renderingConfig.colorRenderTargets[0].clearValue = {
            surfaceConfig.clearColor.r,
            surfaceConfig.clearColor.g,
            surfaceConfig.clearColor.b,
            1.0f};
    }

    void ForwardRenderer::render(
        const uint32_t frameIndex,
        const vireo::Extent& extent,
        const std::shared_ptr<vireo::Semaphore>& renderingFinishedSemaphore) {
        const auto& frame = framesData[frameIndex];
        const auto& frameMeshes = MeshesRenderer::framesData[frameIndex];

        renderingConfig.colorRenderTargets[0].renderTarget = frame.colorAttachment;

        frameMeshes.commandAllocator->reset();
        const auto commandList = frameMeshes.commandList;
        commandList->begin();
        commandList->barrier(frame.colorAttachment, vireo::ResourceState::UNDEFINED,vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList->beginRendering(renderingConfig);
        commandList->setViewport(extent);
        commandList->setScissors(extent);
        commandList->endRendering();
        commandList->end();

        submitQueue->submit(vireo::WaitStage::COLOR_OUTPUT, renderingFinishedSemaphore, {commandList});
    }

    void ForwardRenderer::resize(const vireo::Extent& extent) {
        for (auto& frame : framesData) {
            frame.colorAttachment = vireo->createRenderTarget(
                surfaceConfig.renderingFormat,
                extent.width, extent.height,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[0].clearValue);
        }
    }
}