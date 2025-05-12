/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.forward_color;

namespace lysa {
    ForwardColor::ForwardColor(
        const SurfaceConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const Samplers& samplers):
        Renderpass{surfaceConfig, vireo, samplers, L"Forward Color"} {
        pipelineConfig.colorRenderFormats.push_back(surfaceConfig.renderingFormat);
        renderingConfig.colorRenderTargets[0].clearValue = {
            surfaceConfig.clearColor.r,
            surfaceConfig.clearColor.g,
            surfaceConfig.clearColor.b,
            1.0f};
    }

    void ForwardColor::render(
        const uint32_t frameIndex,
        const vireo::Extent& extent,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::CommandList>& commandList) {
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        commandList->beginRendering(renderingConfig);
        commandList->setViewport(extent);
        commandList->setScissors(extent);
        commandList->endRendering();
    }
}