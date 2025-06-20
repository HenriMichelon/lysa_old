/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.deferred_renderer;

import vireo;
import lysa.global;

namespace lysa {
    DeferredRenderer::DeferredRenderer(
        const RenderingConfiguration& config,
        const std::wstring& name) :
        Renderer{config, true, name},
        gBufferPass{config},
        lightingPass{config, gBufferPass},
        forwardColor{config, true} {
    }

    void DeferredRenderer::updatePipelines(
        const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        Renderer::updatePipelines(pipelineIds);
        gBufferPass.updatePipelines(pipelineIds);
        forwardColor.updatePipelines(pipelineIds);
    }

    void DeferredRenderer::colorPass(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool,
        const uint32 frameIndex) {
        gBufferPass.render(commandList, scene, colorAttachment, depthAttachment, false, frameIndex);
        lightingPass.render(commandList, scene, colorAttachment, depthAttachment, true, frameIndex);
        forwardColor.render(commandList, scene, colorAttachment, depthAttachment, false, frameIndex);
    }

    void DeferredRenderer::resize(const vireo::Extent& extent) {
        Renderer::resize(extent);
        gBufferPass.resize(extent);
        forwardColor.resize(extent);
    }

}