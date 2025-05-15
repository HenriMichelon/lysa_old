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
        Renderer{surfaceConfig, vireo, name},
        forwardColorPass{surfaceConfig, vireo, samplers} {
    }

    void ForwardRenderer::update(const uint32 frameIndex) {
        forwardColorPass.update(frameIndex);
    }

    void ForwardRenderer::mainColorPass(
        const uint32 frameIndex,
        Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::CommandList>& commandList) {
        forwardColorPass.render(frameIndex, scene, colorAttachment, commandList, false);
    }

    void ForwardRenderer::resize(const vireo::Extent& extent) {
        Renderer::resize(extent);
        forwardColorPass.resize(extent);
    }

}