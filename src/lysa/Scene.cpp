/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module;
#include <xxhash.h>
module lysa.scene;

import vireo;
import lysa.application;

namespace lysa {
    Scene::Scene(
        const SceneConfiguration& config,
        const uint32 framesInFlight,
        const vireo::Viewport& viewport,
        const vireo::Rect& scissors) :
        framesInFlight{framesInFlight},
        viewport{viewport},
        scissors{scissors},
        sceneUniformBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneData), 1,
            L"Scene Data")},
        instancesDataArray{Application::getVireo(),
            sizeof(MeshSurfaceInstanceData),
            config.maxMeshSurfacePerFrame,
            config.maxMeshSurfacePerFrame,
            vireo::BufferType::DEVICE_STORAGE,
            L"MeshSurface Instance Data"},
        modelsData{config} {
        if (descriptorLayout == nullptr) {
            descriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
            descriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
            descriptorLayout->add(BINDING_INSTANCE_DATA, vireo::DescriptorType::STORAGE);
            descriptorLayout->add(BINDING_INSTANCE_INDEX, vireo::DescriptorType::STORAGE);
            descriptorLayout->build();
        }
        descriptorSet = Application::getVireo().createDescriptorSet(descriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
        descriptorSet->update(BINDING_INSTANCE_DATA, instancesDataArray.getBuffer());
        sceneUniformBuffer->map();
    }

    void Scene::update(const vireo::CommandList& commandList) {
        if (currentCamera && currentCamera->isUpdated()) {
            const auto sceneUniform = SceneData {
                .cameraPosition = currentCamera->getPositionGlobal(),
                .projection = currentCamera->getProjection(),
                .view = inverse(currentCamera->getTransformGlobal()),
                .viewInverse = currentCamera->getTransformGlobal(),
            };
            sceneUniformBuffer->write(&sceneUniform);
            currentCamera->updated--;
        }

        for (const auto& meshInstance : models) {
            if (meshInstance->isUpdated()) {
                const auto& mesh = meshInstance->getMesh();
                const auto& meshSurfaces = mesh->getSurfaces();
                auto instancesData = std::vector<MeshSurfaceInstanceData>{meshSurfaces.size()};
                for (int surfaceIndex = 0; surfaceIndex < meshSurfaces.size(); surfaceIndex++) {
                    const auto& meshSurface = meshSurfaces[surfaceIndex];
                    instancesData[surfaceIndex].transform = meshInstance->getTransformGlobal();
                    instancesData[surfaceIndex].vertexIndex = mesh->getVertexIndex();
                    instancesData[surfaceIndex].materialIndex = meshSurface->material->getMaterialIndex();
                }
                instancesDataArray.write(instancesDataMemoryBlocks[meshInstance], instancesData.data());
                instancesDataUpdated = true;

                for (const auto& material : mesh->getMaterials()) {
                    if (material->isUpdated()) {
                        material->upload();
                        Application::getResources().setUpdated();
                    }
                }
            }
        }

        if (instancesDataUpdated) {
            instancesDataArray.flush(commandList);
            instancesDataUpdated = false;
        }

        if (Application::getResources().isUpdated()) {
            Application::getResources().flush(commandList);
        }

        modelsData.update(commandList);
    }

    void Scene::addNode(const std::shared_ptr<Node>& node) {
        switch (node->getType()) {
        case Node::CAMERA:
            node->framesInFlight = framesInFlight;
            activateCamera(static_pointer_cast<Camera>(node));
            break;
        case Node::MESH_INSTANCE:{
            const auto& meshInstance = static_pointer_cast<MeshInstance>(node);
            const auto& mesh = meshInstance->getMesh();
            assert([&]{return !mesh->getMaterials().empty(); }, "Models without materials are not supported");
            if (!mesh->isUploaded()) {
                mesh->upload();
                Application::getResources().setUpdated();
            }
            models.push_back(meshInstance);
            meshInstance->framesInFlight = framesInFlight;
            meshInstance->setUpdated();

            for (const auto& material : mesh->getMaterials()) {
                material->framesInFlight = framesInFlight;
            }

            instancesDataMemoryBlocks[meshInstance] = modelsData.addNode(meshInstance, instancesDataArray);
            break;
        }
        default:
            break;
        }
    }

    void Scene::removeNode(const std::shared_ptr<Node>& node) {
        switch (node->getType()) {
        case Node::CAMERA:
            if (node == currentCamera) {
                currentCamera->setActive(false);
                currentCamera.reset();
            }
            break;
        case Node::MESH_INSTANCE:{
            const auto& meshInstance = static_pointer_cast<MeshInstance>(node);
            if (instancesDataMemoryBlocks.contains(meshInstance)) {
                const auto& memoryBlock = instancesDataMemoryBlocks[meshInstance];
                instancesDataArray.free(memoryBlock);
                instancesDataMemoryBlocks.erase(meshInstance);
                models.remove(meshInstance);
                modelsData.removeNode(meshInstance, instancesDataMemoryBlocks);
            }
            break;
        }
        default:
            break;
        }
    }

    void Scene::drawOpaquesModels(
        vireo::CommandList& commandList,
        const vireo::Pipeline& pipeline,
        const Samplers& samplers) const {
        if (modelsData.instancesIndex.empty()) { return; }
        descriptorSet->update(BINDING_INSTANCE_INDEX, modelsData.instancesIndexBuffer);
        draw(commandList, pipeline, samplers, modelsData.drawCommandsBuffer);
    }

    void Scene::draw(
        vireo::CommandList& commandList,
        const vireo::Pipeline& pipeline,
        const Samplers& samplers,
        const std::shared_ptr<vireo::Buffer>& drawCommand) const {
        commandList.setViewport(viewport);
        commandList.setScissors(scissors);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors(pipeline, {
            Application::getResources().getDescriptorSet(),
            descriptorSet,
            samplers.getDescriptorSet()});
        commandList.drawIndirect(drawCommand, 0, 1, sizeof(vireo::DrawIndirectCommand));
    }

    void Scene::activateCamera(const std::shared_ptr<Camera>& camera) {
        if (currentCamera != nullptr)
            currentCamera->setActive(false);
        if (camera == nullptr) {
            currentCamera.reset();
        } else {
            currentCamera = camera;
            currentCamera->setActive(true);
        }
    }

    Scene::ModelsData::ModelsData(const SceneConfiguration& config):
        drawCommandsStagingBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::BUFFER_UPLOAD,
            sizeof(vireo::DrawIndexedIndirectCommand), 1,
            L"Draw commands upload")},
        drawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::INDIRECT,
            sizeof(vireo::DrawIndexedIndirectCommand), 1,
            L"Draw commands")},
        instancesIndexBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::STORAGE,
            sizeof(Index) * config.maxVertexPerFrame, 1,
            L"Draw indices")} {
        instancesIndexBuffer->map();
        drawCommandsStagingBuffer->map();
    }

    void Scene::ModelsData::update(const vireo::CommandList& commandList) {
        if (instancesIndexUpdated) {
            if (instancesIndex.size() > 0) {
                const auto drawCommand = vireo::DrawIndirectCommand {
                    .vertexCount = static_cast<uint32>(instancesIndex.size())
                };
                drawCommandsStagingBuffer->write(&drawCommand, sizeof(drawCommand));
                commandList.copy(drawCommandsStagingBuffer, drawCommandsBuffer);
            }
            instancesIndexUpdated = false;
        }
    }

    MemoryBlock Scene::ModelsData::addNode(
        const std::shared_ptr<MeshInstance>& meshInstance,
        DeviceMemoryArray& instancesDataArray) {
        models.push_back(meshInstance);
        const auto& mesh = meshInstance->getMesh();
        const auto& meshSurfaces{mesh->getSurfaces()};
        const auto& meshIndices{mesh->getIndices()};
        const auto memoryBlock{instancesDataArray.alloc(meshSurfaces.size())};
        const auto startIndex{instancesIndex.size()};
        for (int surfaceIndex = 0; surfaceIndex < meshSurfaces.size(); surfaceIndex++) {
            const auto& meshSurface = meshSurfaces[surfaceIndex];
            const auto instanceDataIndex = memoryBlock.instanceIndex + surfaceIndex;
            for (int i = 0; i < meshSurface->indexCount; i++) {
                instancesIndex.push_back({
                   .index = meshIndices[meshSurface->firstIndex + i],
                   .surfaceIndex = instanceDataIndex
                });
            }
        }
        instancesIndexBuffer->write(
            &instancesIndex[startIndex],
            mesh->getIndices().size() * sizeof(Index),
            startIndex * sizeof(Index));
        instancesIndexUpdated = true;
        return memoryBlock;
    }

    void Scene::ModelsData::removeNode(
        const std::shared_ptr<MeshInstance>& meshInstance,
        const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks) {
        models.remove(meshInstance);
        rebuildInstancesIndex(instancesDataMemoryBlocks);
    }

    void Scene::ModelsData::rebuildInstancesIndex(
        const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks) {
        instancesIndex.clear();
        for (const auto& meshInstance : models) {
            const auto& mesh = meshInstance->getMesh();
            const auto& meshSurfaces = mesh->getSurfaces();
            for (int surfaceIndex = 0; surfaceIndex < meshSurfaces.size(); surfaceIndex++) {
                const auto& meshSurface = meshSurfaces[surfaceIndex];
                const auto instanceDataIndex = instancesDataMemoryBlocks.at(meshInstance).instanceIndex + surfaceIndex;
                for (int i = 0; i < meshSurface->indexCount; i++) {
                    instancesIndex.push_back({
                        .index = mesh->getIndices()[meshSurface->firstIndex + i],
                        .surfaceIndex = instanceDataIndex
                    });
                }
            }
        }
        instancesIndexBuffer->write(instancesIndex.data(), instancesIndex.size() * sizeof(Index));
        instancesIndexUpdated = true;
    }

}
