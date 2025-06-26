/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderer;

import lysa.application;

namespace lysa {
    Renderer::Renderer(
        const RenderingConfiguration& config,
        const bool withStencil,
        const std::wstring& name) :
        config{config},
        name{name},
        withStencil{withStencil},
        depthPrePass{config, withStencil},
        shaderMaterialPass{config},
        transparencyPass{config} {
        framesData.resize(config.framesInFlight);
    }

    void Renderer::update(const uint32 frameIndex) {
        for (const auto& shadowMapPass : shadowMapPasses) {
            shadowMapPass->update(frameIndex);
        }
        depthPrePass.update(frameIndex);
        for (const auto& postProcessingPass : postProcessingPasses) {
            postProcessingPass->update(frameIndex);
        }
    }

    void Renderer::updatePipelines(
     const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        depthPrePass.updatePipelines(pipelineIds);
        shaderMaterialPass.updatePipelines(pipelineIds);
        transparencyPass.updatePipelines(pipelineIds);
    }

    void Renderer::update(
        const std::shared_ptr<vireo::CommandList>& commandList,
        Scene& scene) const {
        scene.update(*commandList);
        scene.compute(*commandList);
    }

    void Renderer::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const bool clearAttachment,
        const uint32 frameIndex) {
        auto resourcesLock = std::lock_guard{Application::getResources().getMutex()};
        for (const auto& shadowMapPass : shadowMapPasses) {
            shadowMapPass->render(commandList, scene, frameIndex);
        }
        const auto& frame = framesData[frameIndex];
        scene.setInitialState(commandList);
        depthPrePass.render(commandList, scene, frame.depthAttachment);
        colorPass(commandList, scene, frame.colorAttachment, frame.depthAttachment, clearAttachment, frameIndex);
        shaderMaterialPass.render(commandList, scene, frame.colorAttachment, frame.depthAttachment, false, frameIndex);
        transparencyPass.render(commandList, scene, frame.colorAttachment, frame.depthAttachment, false, frameIndex);
    }

    void Renderer::postprocess(
        vireo::CommandList& commandList,
        const vireo::Viewport&viewport,
        const vireo::Rect&scissor,
        uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];
        if (!postProcessingPasses.empty()) {
            commandList.barrier(
                frame.colorAttachment,
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::SHADER_READ);
            std::ranges::for_each(postProcessingPasses, [&](const auto& postProcessingPass) {
                postProcessingPass->render(
                    frameIndex,
                    viewport,
                    scissor,
                    frame.colorAttachment,
                    commandList,
                    postProcessingPass != postProcessingPasses.back());
            });
            commandList.barrier(
                frame.colorAttachment,
                vireo::ResourceState::SHADER_READ,
                vireo::ResourceState::UNDEFINED);
        }
    }

    std::shared_ptr<vireo::Image> Renderer::getColorImage(const uint32 frameIndex) const {
        if (postProcessingPasses.empty()) {
            return framesData[frameIndex].colorAttachment->getImage();
        }
        return postProcessingPasses.back()->getColorAttachment(frameIndex);
    }

    void Renderer::resize(const vireo::Extent& extent) {
        currentExtent = extent;
        for (auto& frame : framesData) {
            frame.colorAttachment = Application::getVireo().createRenderTarget(
                config.renderingFormat,
                extent.width, extent.height,
                vireo::RenderTargetType::COLOR,
                {config.clearColor.r, config.clearColor.g, config.clearColor.b, 1.0f},
                1,
                config.msaa,
                name);
            frame.depthAttachment = Application::getVireo().createRenderTarget(
                config.depthStencilFormat,
                extent.width, extent.height,
                vireo::RenderTargetType::DEPTH,
                { .depthStencil = { .depth = 1.0f, .stencil = 0 } },
                1,
                config.msaa,
                name + L" DepthStencil");
        }
        transparencyPass.resize(extent);
        for (const auto& postProcessingPass : postProcessingPasses) {
            postProcessingPass->resize(extent);
        }
    }

    void Renderer::addPostprocessing(const std::wstring& fragShaderName, void* data, const uint32 dataSize) {
        const auto postProcessingPass = std::make_shared<PostProcessing>(
            config,
            fragShaderName,
            data,
            dataSize,
            fragShaderName);
        postProcessingPass->resize(currentExtent);
        postProcessingPasses.push_back(postProcessingPass);
    }

    void Renderer::removePostprocessing(const std::wstring& fragShaderName) {
        std::erase_if(postProcessingPasses, [&fragShaderName](const std::shared_ptr<PostProcessing>& item) {
            return item->getFragShaderName() == fragShaderName;
        });
    }

    void Renderer::addShadowMapPass(const std::shared_ptr<ShadowMapPass>& shadowMapPass) {
        shadowMapPasses.push_back(shadowMapPass);
    }

    void Renderer::removeShadowMapPass(const std::shared_ptr<ShadowMapPass>& shadowMapPass) {
        shadowMapPasses.remove(shadowMapPass);
    }


}