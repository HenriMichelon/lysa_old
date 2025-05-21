/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.forward_color;

import lysa.application;
import lysa.resources_manager;
import lysa.resources.mesh;

namespace lysa {
    ForwardColor::ForwardColor(
        const RenderingConfiguration& config,
        const Samplers& samplers):
        Renderpass{config, samplers, L"Forward Color"} {
        pipelineConfig.colorRenderFormats.push_back(config.renderingFormat);
        pipelineConfig.resources = Application::getVireo().createPipelineResources({
            Resources::descriptorLayout,
            Scene::descriptorLayout,
            samplers.getDescriptorLayout()},
            {}, name);
        pipelineConfig.vertexShader = Application::getVireo().createShaderModule("shaders/default.vert");
        pipelineConfig.fragmentShader = Application::getVireo().createShaderModule("shaders/forward.frag");
        pipeline = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
        renderingConfig.colorRenderTargets[0].clearValue = {
            config.clearColor.r,
            config.clearColor.g,
            config.clearColor.b,
            1.0f};
    }

    void ForwardColor::render(
        const uint32 frameIndex,
        Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::CommandList>& commandList,
        const bool recordLastBarrier) {
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        commandList->beginRendering(renderingConfig);
        commandList->setViewport(scene.getViewport());
        commandList->setScissors(scene.getScissors());
        scene.draw(
            commandList,
            pipeline,
            samplers);
        commandList->endRendering();
    }
}