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
        ssaoPass = std::make_unique<SSAOPass>(config, gBufferPass);
        ssaoBlurPass = std::make_unique<PostProcessing>(
              config,
              L"ssao_blur",
              ssaoPass->getSSAOBufferFormat(),
              &blurData,
              sizeof(blurData),
              L"SSAO Blur");
    }

    void DeferredRenderer::update(const uint32 frameIndex) {
        Renderer::update(frameIndex);
        ssaoBlurPass->update(frameIndex);
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
        ssaoPass->render(commandList, scene, frameIndex);
        ssaoBlurPass->render(
               frameIndex,
               scene.getViewport(),
               scene.getScissors(),
                ssaoPass->getSSAOColorBuffer(frameIndex),
               nullptr,
               nullptr,
               commandList);
        lightingPass.render(
            commandList,
            scene,
            colorAttachment,
            depthAttachment,
            ssaoBlurPass->getColorAttachment(frameIndex),
            true,
            frameIndex);
    }

    void DeferredRenderer::resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) {
        Renderer::resize(extent, commandList);
        gBufferPass.resize(extent, commandList);
        ssaoPass->resize(extent, commandList);
        ssaoBlurPass->resize(extent, commandList);
        lightingPass.resize(extent, commandList);
    }

}