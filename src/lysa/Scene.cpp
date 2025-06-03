/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.scene;

import vireo;
import lysa.application;
import lysa.log;

namespace lysa {

    void Scene::createDescriptorLayouts() {
        sceneDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
        sceneDescriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
        sceneDescriptorLayout->add(BINDING_INSTANCE_DATA, vireo::DescriptorType::STORAGE);
        sceneDescriptorLayout->add(BINDING_LIGHTS, vireo::DescriptorType::UNIFORM);
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
        lightsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(LightData),
            1,
            L"Scene Lights")},
        instancesDataArray{Application::getVireo(),
            sizeof(MeshSurfaceInstanceData),
            config.maxMeshSurfacePerFrame,
            config.maxMeshSurfacePerFrame,
            vireo::BufferType::DEVICE_STORAGE,
            L"MeshSurface Instance Data"},
        sceneUniformBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneData), 1,
            L"Scene Data")},
        scissors{scissors},
        viewport{viewport},
        framesInFlight{framesInFlight} {
        descriptorSet = Application::getVireo().createDescriptorSet(sceneDescriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
        descriptorSet->update(BINDING_INSTANCE_DATA, instancesDataArray.getBuffer());
        descriptorSet->update(BINDING_LIGHTS, lightsBuffer);
        sceneUniformBuffer->map();
        lightsBuffer->map();

        opaquePipelinesData[DEFAULT_PIPELINE_ID] = std::make_unique<PipelineData>(PipelineData{config, DEFAULT_PIPELINE_ID});
    }

    void Scene::update(const vireo::CommandList& commandList) {
        instancesDataArray.restart();
        auto sceneUniform = SceneData {
            .cameraPosition = currentCamera->getPositionGlobal(),
            .projection = currentCamera->getProjection(),
            .view = inverse(currentCamera->getTransformGlobal()),
            .viewInverse = currentCamera->getTransformGlobal(),
            .lightsCount = static_cast<uint32>(lights.size())
        };
        if (currentEnvironment) {
            sceneUniform.ambientLight = currentEnvironment->getAmbientColorAndIntensity();
        }
        sceneUniformBuffer->write(&sceneUniform);

        for (const auto& meshInstance : models) {
            const auto& mesh = meshInstance->getMesh();
            if (meshInstance->isUpdated() || mesh->isUpdated()) {
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
                if (meshInstance->isUpdated()) { meshInstance->updated--; }
                if (mesh->isUpdated()) { mesh->updated--; }
            }
            for (const auto& material : mesh->getMaterials()) {
                if (material->isUpdated()) {
                    material->upload();
                    Application::getResources().setUpdated();
                }
            }
        }
        if (instancesDataUpdated) {
            instancesDataArray.flush(commandList);
            instancesDataUpdated = false;
        }

        for (auto& [pipelineId, pipelineData] : opaquePipelinesData) {
            pipelineData->update(commandList);
        }

        if (Application::getResources().isUpdated()) {
            Application::getResources().flush(commandList);
        }

        if (!lights.empty()) {
            if (lights.size() > lightsBufferCount) {
                if (lightsBufferCount >= Light::MAX_LIGHTS) {
                    throw Exception("Too many lights");
                }
                lightsBufferCount = lights.size();
                lightsBuffer = Application::getVireo().createBuffer(
                    vireo::BufferType::UNIFORM,
                    sizeof(LightData) * lightsBufferCount, 1,
                    L"Scene Lights");
                lightsBuffer->map();
                descriptorSet->update(BINDING_LIGHTS, lightsBuffer);
            }
            auto lightIndex = 0;
            auto lightsArray = std::vector<LightData>(lightsBufferCount);
            for (const auto& light : lights) {
                // if (!light->isVisible()) { continue; }
                lightsArray[lightIndex] = light->getLightData();
                lightIndex++;
            }
            lightsBuffer->write(lightsArray.data(), lightsArray.size() * sizeof(LightData));
        }

    }

    void Scene::addNode(const std::shared_ptr<Node>& node) {
        switch (node->getType()) {
        case Node::CAMERA:
            node->framesInFlight = framesInFlight;
            activateCamera(static_pointer_cast<Camera>(node));
            break;
        case Node::ENVIRONMENT:
            node->framesInFlight = framesInFlight;
            currentEnvironment = static_pointer_cast<Environment>(node);
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
            mesh->framesInFlight = framesInFlight;
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
                if (!opaquePipelinesData.contains(pipelineId)) {
                    opaquePipelinesData[pipelineId] = std::make_unique<PipelineData>(PipelineData{config, pipelineId});
                }
                opaquePipelinesData[pipelineId]->addNode(meshInstance, instancesDataArray, instancesDataMemoryBlocks);
            }
            break;
        }
        case Node::DIRECTIONAL_LIGHT:
            lights.push_back(static_pointer_cast<Light>(node));
            break;
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
        case Node::ENVIRONMENT:
            if (node == currentEnvironment) {
                currentEnvironment.reset();
            }
            break;
        case Node::MESH_INSTANCE:{
            const auto& meshInstance = static_pointer_cast<MeshInstance>(node);
            if (instancesDataMemoryBlocks.contains(meshInstance)) {
                const auto& mesh = meshInstance->getMesh();
                const auto& memoryBlock = instancesDataMemoryBlocks[meshInstance];
                instancesDataArray.free(memoryBlock);
                instancesDataMemoryBlocks.erase(meshInstance);
                models.remove(meshInstance);
                auto nodePipelineIds = std::list<uint32>{};
                for (const auto& material : mesh->getMaterials()) {
                    auto id = material->getPipelineId();
                    nodePipelineIds.push_back(id);
                }
                for (const auto& pipelineId : nodePipelineIds) {
                    opaquePipelinesData[pipelineId]->removeNode(meshInstance, instancesDataMemoryBlocks);
                }
            }
            break;
        }
        case Node::DIRECTIONAL_LIGHT:
            lights.remove(static_pointer_cast<Light>(node));
            break;
        default:
            break;
        }
    }

    void Scene::drawOpaquesModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const {
        commandList.setViewport(viewport);
        commandList.setScissors(scissors);
        for (const auto& [pipelineId, pipelineData] : opaquePipelinesData) {
            if (!pipelineData->instancesIndex.empty()) {
                const auto& pipeline = pipelines.at(pipelineId);
                commandList.bindPipeline(pipeline);
                commandList.bindDescriptors(pipeline, {
                    Application::getResources().getDescriptorSet(),
                    Application::getResources().getSamplers().getDescriptorSet(),
                    descriptorSet,
                    pipelineData->descriptorSet,
                });
                commandList.drawIndirect(pipelineData->drawCommandsBuffer, 0, 1, sizeof(vireo::DrawIndirectCommand));
            }
        }
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

    Scene::PipelineData::PipelineData(const SceneConfiguration& config, const uint32 pipelineId):
        config{config},
        pipelineId{pipelineId},
        drawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::INDIRECT,
            sizeof(vireo::DrawIndexedIndirectCommand), 1,
            L"Draw commands")},
        drawCommandsStagingBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::BUFFER_UPLOAD,
            sizeof(vireo::DrawIndexedIndirectCommand), 1,
            L"Draw commands upload")},
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

    void Scene::PipelineData::update(const vireo::CommandList& commandList) {
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

    void Scene::PipelineData::addNode(
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
                if (meshSurface->material->getPipelineId() == pipelineId) {
                    const auto instanceDataIndex = memoryBlock.instanceIndex + surfaceIndex;
                    for (int i = 0; i < meshSurface->indexCount; i++) {
                        instancesIndex.push_back({
                           .index = meshIndices[meshSurface->firstIndex + i],
                           .surfaceIndex = instanceDataIndex
                        });
                    }
                    surfaceIndex++;
                }
            }
            if ((startIndex + mesh->getIndices().size() > config.maxVertexPerFrame)) {
                throw Exception("Too many vertices in a single frame");
            }
            instancesIndexBuffer->write(
                &instancesIndex[startIndex],
                mesh->getIndices().size() * sizeof(Index),
                startIndex * sizeof(Index));
            instancesIndexUpdated = true;
            instancesDataMemoryBlocks[meshInstance] = memoryBlock;
        }
    }

    void Scene::PipelineData::removeNode(
        const std::shared_ptr<MeshInstance>& meshInstance,
        const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks) {
        models.remove(meshInstance);
        rebuildInstancesIndex(instancesDataMemoryBlocks);
    }

    void Scene::PipelineData::rebuildInstancesIndex(
        const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks) {
        instancesIndex.clear();
        for (const auto& meshInstance : models) {
            const auto& mesh = meshInstance->getMesh();
            const auto& meshSurfaces = mesh->getSurfaces();

            auto surfaceCount{0};
            for (const auto& meshSurface : meshSurfaces) {
                if (meshSurface->material->getPipelineId() == pipelineId) {
                    surfaceCount++;
                }
            }

            if (surfaceCount > 0) {
                auto surfaceIndex{0};
                for (const auto& meshSurface : meshSurfaces) {
                    if (meshSurface->material->getPipelineId() == pipelineId) {
                        const auto instanceDataIndex = instancesDataMemoryBlocks.at(meshInstance).instanceIndex + surfaceIndex;
                        for (int i = 0; i < meshSurface->indexCount; i++) {
                            instancesIndex.push_back({
                                .index = mesh->getIndices()[meshSurface->firstIndex + i],
                                .surfaceIndex = instanceDataIndex
                            });
                        }
                        surfaceIndex++;
                    }
                }
            }
        }
        instancesIndexBuffer->write(instancesIndex.data(), instancesIndex.size() * sizeof(Index));
        instancesIndexUpdated = true;
    }

}
