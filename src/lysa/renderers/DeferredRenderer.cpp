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
        Renderer{config, name},
        gBufferPass{config},
        gLightingPass{config, gBufferPass} {
    }

    void DeferredRenderer::updatePipelines(
        const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        Renderer::updatePipelines(pipelineIds);
        gBufferPass.updatePipelines(pipelineIds);
        gLightingPass.updatePipelines(pipelineIds);
    }

    void DeferredRenderer::mainColorPass(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool clearAttachment,
        const uint32 frameIndex) {
        gBufferPass.render(commandList, scene, colorAttachment, depthAttachment, clearAttachment, frameIndex);
        gLightingPass.render(commandList, scene, colorAttachment, depthAttachment, clearAttachment, frameIndex);
    }

    void DeferredRenderer::resize(const vireo::Extent& extent) {
        Renderer::resize(extent);
        gBufferPass.resize(extent);
    }

}