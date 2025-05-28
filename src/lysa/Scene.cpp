/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.scene;

import vireo;
import lysa.application;

namespace lysa {

    void Scene::createDescriptorLayouts() {
        sceneDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
        sceneDescriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
        sceneDescriptorLayout->add(BINDING_INSTANCE_DATA, vireo::DescriptorType::STORAGE);
        sceneDescriptorLayout->build();
        drawCommandDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Draw Command");
        drawCommandDescriptorLayout->add(BINDING_INSTANCE_INDEX, vireo::DescriptorType::STORAGE);
        drawCommandDescriptorLayout->build();
    }

    void Scene::destroyDescriptorLayouts() {
        sceneDescriptorLayout.reset();
        drawCommandDescriptorLayout.reset();
    }

    Scene::Scene(
        const SceneConfiguration& config,
        const uint32 framesInFlight,
        const vireo::Viewport& viewport,
        const vireo::Rect& scissors) :
        config{config},
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
            L"MeshSurface Instance Data"} {
        descriptorSet = Application::getVireo().createDescriptorSet(sceneDescriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
        descriptorSet->update(BINDING_INSTANCE_DATA, instancesDataArray.getBuffer());
        sceneUniformBuffer->map();

        opaqueModels[DEFAULT_PIPELINE_ID] = std::make_unique<ModelsData>(ModelsData{config, DEFAULT_PIPELINE_ID});
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

        for (auto& [pipelineId, modelsData] : opaqueModels) {
            modelsData->update(commandList);
        }
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

            auto nodePipelineIds = std::list<uint32>{};
            for (const auto& material : mesh->getMaterials()) {
                material->framesInFlight = framesInFlight;
                auto id = material->getPipelineId();
                nodePipelineIds.push_back(id);
                if (!materials.contains(id)) {
                    materials[id] = material;
                    materialsUpdated = true;
                }
            }

            for (const auto& pipelineId : nodePipelineIds) {
                if (!opaqueModels.contains(pipelineId)) {
                    opaqueModels[pipelineId] = std::make_unique<ModelsData>(ModelsData{config, pipelineId});
                }
                opaqueModels[pipelineId]->addNode(meshInstance, instancesDataArray, instancesDataMemoryBlocks);
            }

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
                // opaqueModels.removeNode(meshInstance, instancesDataMemoryBlocks);
            }
            break;
        }
        default:
            break;
        }
    }

    void Scene::drawOpaquesModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines,
        const Samplers& samplers) const {
        commandList.setViewport(viewport);
        commandList.setScissors(scissors);
        commandList.bindDescriptors(pipelines.at(0), {
            Application::getResources().getDescriptorSet(),
            samplers.getDescriptorSet(),
            descriptorSet
        });
        for (const auto& [pipelineId, modelsData] : opaqueModels) {
            if (modelsData->instancesIndex.empty()) { return; }
            commandList.bindPipeline(pipelines.at(pipelineId));
            commandList.bindDescriptor(
                pipelines.at(pipelineId),
                modelsData->descriptorSet,
                SET_DRAW_COMMAND);
            commandList.drawIndirect( modelsData->drawCommandsBuffer, 0, 1, sizeof(vireo::DrawIndirectCommand));
            // draw(commandList, pipeline, samplers, modelsData->drawCommandsBuffer);
        }
    }

    // void Scene::draw(
        // vireo::CommandList& commandList,
        // const vireo::Pipeline& pipeline,
        // const Samplers& samplers,
        // const std::shared_ptr<vireo::Buffer>& drawCommand) const {
    // }

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

    Scene::ModelsData::ModelsData(const SceneConfiguration& config, const uint32 pipelineId):
        pipelineId{pipelineId},
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
        descriptorSet = Application::getVireo().createDescriptorSet(
            drawCommandDescriptorLayout,
            L"Draw Command");
        descriptorSet->update(BINDING_INSTANCE_INDEX, instancesIndexBuffer);
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

    void Scene::ModelsData::addNode(
        const std::shared_ptr<MeshInstance>& meshInstance,
        DeviceMemoryArray& instancesDataArray,
        std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks) {
        models.push_back(meshInstance);
        const auto& mesh = meshInstance->getMesh();
        const auto& meshSurfaces{mesh->getSurfaces()};
        const auto& meshIndices{mesh->getIndices()};

        auto surfaceCount{0};
        for (const auto& meshSurface : meshSurfaces) {
            if (meshSurface->material->getPipelineId() == pipelineId) {
                surfaceCount++;
            }
        }

        if (surfaceCount > 0) {
            const auto memoryBlock{instancesDataArray.alloc(surfaceCount)};
            const auto startIndex{instancesIndex.size()};
            auto surfaceIndex{0};
            for (const auto& meshSurface : meshSurfaces) {
                const auto instanceDataIndex = memoryBlock.instanceIndex + surfaceIndex;
                for (int i = 0; i < meshSurface->indexCount; i++) {
                    instancesIndex.push_back({
                       .index = meshIndices[meshSurface->firstIndex + i],
                       .surfaceIndex = instanceDataIndex
                    });
                }
                surfaceIndex++;
            }
            instancesIndexBuffer->write(
                &instancesIndex[startIndex],
                mesh->getIndices().size() * sizeof(Index),
                startIndex * sizeof(Index));
            instancesIndexUpdated = true;
            instancesDataMemoryBlocks[meshInstance] = memoryBlock;
        }
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
