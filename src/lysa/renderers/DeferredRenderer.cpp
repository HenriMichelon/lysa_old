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
        Renderer{config, name} {
    }

    void DeferredRenderer::update(const uint32 frameIndex) {
        Renderer::update(frameIndex);
        // forwardColorPass.update(frameIndex);
    }

    void DeferredRenderer::updatePipelines(
        const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        Renderer::updatePipelines(pipelineIds);
        // forwardColorPass.updatePipelines(pipelineIds);
    }

    void DeferredRenderer::mainColorPass(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const bool clearAttachment,
        const uint32 frameIndex) {
        // forwardColorPass.render(commandList, scene, colorAttachment, depthAttachment, clearAttachment, frameIndex);
    }

    void DeferredRenderer::resize(const vireo::Extent& extent) {
        Renderer::resize(extent);
        // forwardColorPass.resize(extent);
    }

}