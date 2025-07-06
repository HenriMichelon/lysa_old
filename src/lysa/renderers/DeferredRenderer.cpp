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
        ssaoBlurData{.kernelSize = config.ssaoBlurKernelSize},
        gBufferPass{config},
        lightingPass{config, gBufferPass} {
        if (config.ssaoEnabled) {
            ssaoPass = std::make_unique<SSAOPass>(config, gBufferPass);
            ssaoBlurPass = std::make_unique<PostProcessing>(
                  config,
                  L"ssao_blur",
                  ssaoPass->getSSAOBufferFormat(),
                  &ssaoBlurData,
                  sizeof(ssaoBlurData),
                  L"SSAO Blur");
        }
    }

    void DeferredRenderer::update(const uint32 frameIndex) {
        Renderer::update(frameIndex);
        if (config.ssaoEnabled) { ssaoBlurPass->update(frameIndex); }
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
        if (config.ssaoEnabled) {
            ssaoPass->render(commandList, scene, depthAttachment, frameIndex);
            ssaoBlurPass->render(
                   frameIndex,
                   scene.getViewport(),
                   scene.getScissors(),
                    ssaoPass->getSSAOColorBuffer(frameIndex),
                   nullptr,
                   nullptr,
                   commandList);
        }
        lightingPass.render(
            commandList,
            scene,
            colorAttachment,
            depthAttachment,
            config.ssaoEnabled ? ssaoBlurPass->getColorAttachment(frameIndex) : nullptr,
            true,
            frameIndex);
        if (config.ssaoEnabled) {
            commandList.barrier(
                ssaoBlurPass->getColorAttachment(frameIndex),
                vireo::ResourceState::SHADER_READ,
                vireo::ResourceState::UNDEFINED);
        }
    }

    void DeferredRenderer::resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) {
        Renderer::resize(extent, commandList);
        gBufferPass.resize(extent, commandList);
        if (config.ssaoEnabled) {
            updateBlurData(ssaoBlurData, extent, 1.2);
            ssaoPass->resize(extent, commandList);
            ssaoBlurPass->resize(extent, commandList);
        }
        lightingPass.resize(extent, commandList);
    }

}