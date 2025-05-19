/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.renderers.scene_data;

import lysa.application;
import lysa.nodes.node;
import lysa.resources.mesh;

namespace lysa {

    SceneData::SceneData(const RenderingConfiguration& config, const vireo::Extent &extent):
        Scene{config, extent}  {
        if (globalDescriptorLayout == nullptr) {
            globalDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
            globalDescriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
            // descriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::STORAGE);
            globalDescriptorLayout->add(BINDING_MATERIALS, vireo::DescriptorType::STORAGE);
            globalDescriptorLayout->build();

            perBufferPairDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene per buffer");
            perBufferPairDescriptorLayout->add(BINDING_INSTANCES_DATA, vireo::DescriptorType::STORAGE);
            perBufferPairDescriptorLayout->build();
        }

        sceneUniformBuffer = Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneUniform), 1,
            L"Scene Uniform");
        sceneUniformBuffer->map();
        // modelsStorageBuffer = Application::getVireo().createBuffer(
            // vireo::BufferType::STORAGE,
            // sizeof(ModelUniform) * config.memoryConfig.maxModelsCount, 1,
            // L"Models Uniforms");
        // modelsStorageBuffer->map();
        materialsStorageBuffer = Application::getVireo().createBuffer(
            vireo::BufferType::STORAGE,
            sizeof(MaterialUniform) * 1000, 1,
            L"Materials Uniforms");
        materialsStorageBuffer->map();

        globalDescriptorSet = Application::getVireo().createDescriptorSet(globalDescriptorLayout, L"Scene");
        globalDescriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
        // descriptorSet->update(BINDING_MODELS, modelsStorageBuffer);
        globalDescriptorSet->update(BINDING_MATERIALS, materialsStorageBuffer);
    }

    void SceneData::update() {
        if (currentCamera && currentCamera->isUpdated()) {
            const auto sceneUniform = SceneUniform {
                .cameraPosition = currentCamera->getPositionGlobal(),
                .projection = currentCamera->getProjection(),
                .view = inverse(currentCamera->getTransformGlobal()),
                .viewInverse = currentCamera->getTransformGlobal(),
            };
            sceneUniformBuffer->write(&sceneUniform);
            currentCamera->updated--;
        }

        for (const auto& [bufferPair, models] :opaqueModels) {
            auto buffer = std::shared_ptr<vireo::Buffer>{};
            auto data = std::vector<InstanceData>{};
            if (!perBufferDescriptorSets.contains(bufferPair)) {
                data = std::vector<InstanceData>(1000);
                buffer = Application::getVireo().createBuffer(
                    vireo::BufferType::STORAGE,
                    sizeof(InstanceData) * 1000, 1,
                    L"Per buffer instances data");
                buffer->map();
                const auto set = Application::getVireo().createDescriptorSet(
                     perBufferPairDescriptorLayout,
                     L"Scene per buffer");
                set->update(BINDING_INSTANCES_DATA, buffer);
                perBufferInstancesDataBuffer[bufferPair] = buffer;
                perBufferDescriptorSets[bufferPair] = set;
                perBufferInstancesData[bufferPair] = data;
            } else {
                buffer = perBufferInstancesDataBuffer[bufferPair];
                data = perBufferInstancesData[bufferPair];
            }
            auto surfaceIndex{0};
            for (const auto& meshInstance : models) {
                if (meshInstance->isUpdated()) {
                    auto subIndex{0};
                    for (const auto& meshSurface : meshInstance->getMesh()->getSurfaces()) {
                        const auto index = surfaceIndex+subIndex;
                        auto& instanceData = data[index];
                        instanceData.transform = meshInstance->getTransformGlobal();
                        instanceData.materialIndex = getMaterialIndex(meshSurface->material->getId());
                        buffer->write(&instanceData, sizeof(instanceData), sizeof(instanceData) * index);
                        subIndex++;
                    }
                    meshInstance->updated--;
                }
                surfaceIndex += meshInstance->getMesh()->getSurfaces().size();
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
                materialsStorageBuffer->write(
                    &materialUniform,
                    sizeof(MaterialUniform),
                    sizeof(MaterialUniform) * materialIndex);
                material->updated--;
            }
        }
    }

    void SceneData::draw(
        const std::shared_ptr<vireo::CommandList>& commandList,
        const std::shared_ptr<vireo::Pipeline>& pipeline,
        const std::unordered_map<BufferPair, std::vector<vireo::DrawIndexedIndirectCommand>>& commandsByBuffer,
        const std::unordered_map<BufferPair, std::shared_ptr<vireo::Buffer>>& commandsBufferByBuffer) const {
        commandList->setDescriptors({globalDescriptorSet});
        commandList->bindDescriptor(pipeline, globalDescriptorSet, SET_GLOBAL);
        for (const auto& [bufferPair, commands] : commandsByBuffer) {
            const auto set = perBufferDescriptorSets.at(bufferPair);
            commandList->setDescriptors({set});
            commandList->bindDescriptor(pipeline, set, SET_PERBUFFER);
            commandList->bindIndexBuffer(bufferPair.indexBuffer);
            commandList->bindVertexBuffer(bufferPair.vertexBuffer   );
            // for (const auto& command : commands) {
            //     commandList->drawIndexed(
            //         command.indexCount,
            //         1,
            //         command.firstIndex,
            //         command.vertexOffset ,
            //         0);
            // }
            commandList->drawIndexedIndirect(
                commandsBufferByBuffer.at(bufferPair),
                0,
                commands.size(),
                sizeof(vireo::DrawIndexedIndirectCommand));
        }
    }

}