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
        lightingPass{config, gBufferPass} {
    }

    void DeferredRenderer::updatePipelines(
        const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        gBufferPass.updatePipelines(pipelineIds);
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
    }

    void DeferredRenderer::resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) {
        Renderer::resize(extent, commandList);
        gBufferPass.resize(extent, commandList);
        lightingPass.resize(extent, commandList);
    }

}