/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.forward_color;

import lysa.resources.mesh;

namespace lysa {
    ForwardColor::ForwardColor(
        const WindowConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const Samplers& samplers):
        Renderpass{surfaceConfig, vireo, samplers, L"Forward Color"} {
        pipelineConfig.colorRenderFormats.push_back(surfaceConfig.renderingFormat);
        pipelineConfig.resources = vireo->createPipelineResources(
            {SceneData::getDescriptorLayout()},
            SceneData::pushConstantsDesc);
        pipelineConfig.vertexInputLayout = vireo->createVertexLayout(sizeof(Vertex), Mesh::vertexAttributes);
        pipelineConfig.vertexShader = vireo->createShaderModule("shaders/default.vert");
        pipelineConfig.fragmentShader = vireo->createShaderModule("shaders/forward.frag");
        pipeline = vireo->createGraphicPipeline(pipelineConfig, name);
        renderingConfig.colorRenderTargets[0].clearValue = {
            surfaceConfig.clearColor.r,
            surfaceConfig.clearColor.g,
            surfaceConfig.clearColor.b,
            1.0f};
    }

    void ForwardColor::render(
        const uint32 frameIndex,
        SceneData& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::CommandList>& commandList,
        const bool recordLastBarrier) {
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        commandList->beginRendering(renderingConfig);
        commandList->setViewport(scene.getViewport());
        commandList->setScissors(scene.getScissors());
        commandList->setDescriptors({scene.getDescriptorSet()});
        commandList->bindPipeline(pipeline);
        commandList->bindDescriptors(pipeline, {scene.getDescriptorSet()});
        uint32 modelIndex{0};
        for (const auto& meshInstance : scene.getModels()) {
            const auto &mesh = meshInstance->getMesh();
            commandList->bindVertexBuffer(mesh->getVertexBuffer());
            commandList->bindIndexBuffer(mesh->getIndexBuffer());
            for (const auto& meshSurface : mesh->getSurfaces()) {
                const auto pushConstants = PushConstants {
                    .modelIndex = modelIndex,
                    .materialIndex = scene.getMaterialIndex(meshSurface->material->getId()),
                };
                commandList->pushConstants(pipelineConfig.resources, SceneData::pushConstantsDesc, &pushConstants);
                commandList->drawIndexed(
                    meshSurface->indexCount,
                    1,
                    meshSurface->firstVertexIndex,
                    0,
                    0);
            }
            modelIndex++;
        }
        commandList->endRendering();
    }
}