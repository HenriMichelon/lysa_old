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
        const RenderingConfiguration& config,
        const std::wstring& name) :
        Renderer{config, false, name},
        forwardColorPass{config} {
    }

    void ForwardRenderer::update(const uint32 frameIndex) {
        Renderer::update(frameIndex);
        forwardColorPass.update(frameIndex);
    }

    void ForwardRenderer::updatePipelines(
        const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        Renderer::updatePipelines(pipelineIds);
        forwardColorPass.updatePipelines(pipelineIds);
    }

    void ForwardRenderer::mainColorPass(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool clearAttachment,
        const uint32 frameIndex) {
        forwardColorPass.render(commandList, scene, colorAttachment, depthAttachment, clearAttachment, frameIndex);
    }


}