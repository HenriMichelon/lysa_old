/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.forward_color;

import lysa.application;
import lysa.resources;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {
    ForwardColor::ForwardColor(
        const RenderingConfiguration& config,
        const Samplers& samplers):
        Renderpass{config, samplers, L"Forward Color"} {
        pipelineConfig.colorRenderFormats.push_back(config.renderingFormat);
        pipelineConfig.resources = Application::getVireo().createPipelineResources({
            Resources::descriptorLayout,
            samplers.getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            Scene::drawCommandDescriptorLayout},
            {}, name);
        pipelineConfig.vertexShader = Application::getVireo().createShaderModule("shaders/" + DEFAULT_VERTEX_SHADER);
        pipelineConfig.fragmentShader = Application::getVireo().createShaderModule("shaders/" + DEFAULT_FRAGMENT_SHADER);
        defaultPipeline = Application::getVireo().createGraphicPipeline(pipelineConfig, name);
        renderingConfig.colorRenderTargets[0].clearValue = {
            config.clearColor.r,
            config.clearColor.g,
            config.clearColor.b,
            1.0f};
    }

    void ForwardColor::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const bool clearAttachment,
        const uint32) {
        renderingConfig.colorRenderTargets[0].clear = clearAttachment;
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        commandList.beginRendering(renderingConfig);
        scene.drawOpaquesModels(
            commandList,
            *defaultPipeline,
            samplers);
        commandList.endRendering();
    }
}