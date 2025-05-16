/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.renderers.scene_data;

import lysa.nodes.node;

namespace lysa {

    SceneData::SceneData(const std::shared_ptr<vireo::Vireo>& vireo, const uint32 framesInFlight, const vireo::Extent &extent):
        Scene{framesInFlight, extent},
        vireo{vireo} {
        if (descriptorLayout == nullptr) {
            descriptorLayout = vireo->createDescriptorLayout(L"Scene");
            descriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
            descriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::UNIFORM);
            descriptorLayout->add(BINDING_MATERIALS, vireo::DescriptorType::UNIFORM);
            descriptorLayout->build();
        }

        sceneUniformBuffer = vireo->createBuffer(vireo::BufferType::UNIFORM, sizeof(SceneUniform), 1, L"Scene Uniform");
        sceneUniformBuffer->map();

        descriptorSet = vireo->createDescriptorSet(descriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
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
        if (modelsUpdated && modelUniformsSize < models.size()) {
            modelUniformsSize = models.size();
            modelUniforms = std::make_unique<ModelUniform[]>(models.size());
            modelUniformBuffers = vireo->createBuffer(vireo::BufferType::UNIFORM, sizeof(ModelUniform) * modelUniformsSize, 1, L"Models Uniform");
            descriptorSet->update(BINDING_MODELS, modelUniformBuffers);
            modelUniformBuffers->map();
            uint32_t modelIndex = 0;
            for (const auto &meshInstance : models) {
                modelUniforms[modelIndex].transform = meshInstance->getTransformGlobal();
                meshInstance->updated = 0;
                modelIndex++;
            }
            modelUniformBuffers->write(modelUniforms.get());
            modelsUpdated = false;
        } else {
            uint32_t modelIndex = 0;
            for (const auto &meshInstance : models) {
                if (meshInstance->isUpdated()) {
                    modelUniforms[modelIndex].transform = meshInstance->getTransformGlobal();
                    modelUniformBuffers->write(&modelUniforms[modelIndex], sizeof(ModelUniform), sizeof(ModelUniform) * modelIndex);
                    meshInstance->updated--;
                }
                modelIndex++;
            }
        }

         // Update in GPU memory only the materials modified since the last frame
        if (materialsUpdated && materialUniformsSize < materials.size()) {
            materialUniformsSize = materials.size();
            materialUniformBuffers = vireo->createBuffer(vireo::BufferType::UNIFORM, sizeof(MaterialUniform) * materialUniformsSize, 1, L"Materials Uniform");
            descriptorSet->update(BINDING_MATERIALS, materialUniformBuffers);
            materialUniformBuffers->map();
        }
        uint32_t materialIndex = 0;
        for (const auto& material : materials) {
            if (materialsUpdated || material->isUpdated()) {
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
            materialIndex++;
        }
        materialsUpdated = false;
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
                const auto& mesh = meshInstance->getMesh();
                for (const auto& meshSurface : mesh->getSurfaces()) {
                    pushConstants.modelIndex = getModelIndex(meshInstance->getId());
                    pushConstants.materialIndex = getMaterialIndex(meshSurface->material->getId());
                    commandList->pushConstants(pipelineResources, pushConstantsDesc, &pushConstants);
                    commandList->drawIndexed(
                        meshSurface->indexCount,
                        1,
                        mesh->getFirstIndex() + meshSurface->firstIndex,
                        mesh->getFirstVertex(),
                        0);
                }
            }
        }
    }

}