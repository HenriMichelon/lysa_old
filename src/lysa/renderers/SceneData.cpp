/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.renderers.scene_data;

import lysa.nodes.node;

namespace lysa {

    SceneData::SceneData(const RenderingConfiguration& config, const std::shared_ptr<vireo::Vireo>& vireo, const vireo::Extent &extent):
        Scene{config, extent},
        vireo{vireo} {
        if (descriptorLayout == nullptr) {
            descriptorLayout = vireo->createDescriptorLayout(L"Scene");
            descriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
            descriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::STORAGE);
            descriptorLayout->add(BINDING_MATERIALS, vireo::DescriptorType::STORAGE);
            descriptorLayout->build();
        }

        sceneUniformBuffer = vireo->createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneUniform), 1,
            L"Scene Uniform");
        sceneUniformBuffer->map();
        modelUniformBuffers = vireo->createBuffer(
            vireo::BufferType::STORAGE,
            sizeof(ModelUniform) * config.memoryConfig.maxModelsCount, 1,
            L"Models Uniforms");
        modelUniformBuffers->map();
        materialUniformBuffers = vireo->createBuffer(
            vireo::BufferType::STORAGE,
            sizeof(MaterialUniform) * config.memoryConfig.maxMaterialsCount, 1,
            L"Materials Uniforms");
        materialUniformBuffers->map();

        descriptorSet = vireo->createDescriptorSet(descriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
        descriptorSet->update(BINDING_MODELS, modelUniformBuffers);
        descriptorSet->update(BINDING_MATERIALS, materialUniformBuffers);
    }

    void SceneData::update() {
        if (currentCamera && currentCamera->isUpdated()) {
            sceneUniform.cameraPosition = currentCamera->getPositionGlobal();
            sceneUniform.projection = currentCamera->getProjection();
            sceneUniform.view = inverse(currentCamera->getTransformGlobal());
            sceneUniform.viewInverse = currentCamera->getTransformGlobal();
            sceneUniformBuffer->write(&sceneUniform);
            currentCamera->updated--;
        }

        // Update in GPU memory only the models modified since the last frame
        auto modelUniform = ModelUniform {};
        for (const auto& modelIndex : std::views::values(modelsIndices)) {
            const auto meshInstance = models[modelIndex];
            if (meshInstance->isUpdated()) {
                modelUniform.transform = meshInstance->getTransformGlobal();
                modelUniformBuffers->write(
                    &modelUniform,
                    sizeof(ModelUniform),
                    sizeof(ModelUniform) * modelIndex);
                meshInstance->updated--;
            }
        }

         // Update in GPU memory only the materials modified since the last frame
        for (const auto& materialIndex : std::views::values(materialsIndices)) {
            const auto material = materials[materialIndex];
            if (material->isUpdated()) {
                auto materialUniform = MaterialUniform {};
                if (const auto *standardMaterial = dynamic_cast<const StandardMaterial *>(material.get())) {
                    materialUniform.albedoColor = standardMaterial->getAlbedoColor();
                }
                materialUniformBuffers->write(
                    &materialUniform,
                    sizeof(MaterialUniform),
                    sizeof(MaterialUniform) * materialIndex);
                material->updated--;
            }
        }
    }

    void SceneData::draw(
         const std::shared_ptr<vireo::CommandList>& commandList,
         const std::shared_ptr<vireo::PipelineResources>& pipelineResources,
         const std::unordered_map<BufferPair, std::list<std::shared_ptr<MeshInstance>>>& modelsByBuffer) const {
        auto pushConstants = PushConstants {};
        for (const auto& bufferPair : std::views::keys(modelsByBuffer)) {
            commandList->bindVertexBuffer(bufferPair.vertexBuffer);
            commandList->bindIndexBuffer(bufferPair.indexBuffer);
            for (const auto& meshInstance : modelsByBuffer.at(bufferPair)) {
                pushConstants.modelIndex = getModelIndex(meshInstance->getId());
                const auto& mesh = meshInstance->getMesh();
                for (const auto& meshSurface : mesh->getSurfaces()) {
                    pushConstants.materialIndex = getMaterialIndex(meshSurface->material->getId());
                    commandList->pushConstants(pipelineResources, pushConstantsDesc, &pushConstants);
                    commandList->drawIndexed(
                        meshSurface->indexCount,
                        1,
                        mesh->getFirstIndex() + meshSurface->firstIndex,
                        mesh->getVertexOffset(),
                        0);
                }
            }
        }
    }

}