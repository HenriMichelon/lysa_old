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
        forwardColorPass{surfaceConfig, vireo, samplers},
        gammaCorrectionPass{surfaceConfig, vireo, samplers} {
        framesData.resize(surfaceConfig.framesInFlight);
    }

    void ForwardRenderer::update(uint32_t frameIndex) {
        forwardColorPass.update(frameIndex);
        gammaCorrectionPass.update(frameIndex);
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
        commandList->barrier(
            frame.colorAttachment,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);
        gammaCorrectionPass.render(frameIndex, extent, frame.colorAttachment, commandList);
        commandList->barrier(
            frame.colorAttachment,
            vireo::ResourceState::SHADER_READ,
            vireo::ResourceState::UNDEFINED);

        commandList->end();
        submitQueue->submit(vireo::WaitStage::COLOR_OUTPUT, renderingFinishedSemaphore, {commandList});
    }

    void ForwardRenderer::resize(const vireo::Extent& extent) {
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
        gammaCorrectionPass.resize(extent);
    }
}