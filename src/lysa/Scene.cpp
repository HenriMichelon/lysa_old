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
        sceneDescriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::DEVICE_STORAGE);
        sceneDescriptorLayout->add(BINDING_LIGHTS, vireo::DescriptorType::UNIFORM);
        sceneDescriptorLayout->build();

        pipelineDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Pipeline");
        pipelineDescriptorLayout->add(BINDING_INSTANCES, vireo::DescriptorType::DEVICE_STORAGE);
        pipelineDescriptorLayout->build();
    }

    void Scene::destroyDescriptorLayouts() {
        sceneDescriptorLayout.reset();
        pipelineDescriptorLayout.reset();
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
        meshInstancesDataArray{Application::getVireo(),
            sizeof(MeshInstanceData),
            config.maxModelsPerFrame,
            config.maxModelsPerFrame,
            vireo::BufferType::DEVICE_STORAGE,
            L"Models Data"},
        sceneUniformBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneData), 1,
            L"Scene Data")},
        scissors{scissors},
        viewport{viewport},
        framesInFlight{framesInFlight} {
        descriptorSet = Application::getVireo().createDescriptorSet(sceneDescriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
        descriptorSet->update(BINDING_MODELS, meshInstancesDataArray.getBuffer());
        descriptorSet->update(BINDING_LIGHTS, lightsBuffer);
        sceneUniformBuffer->map();
        lightsBuffer->map();
    }

    void Scene::compute(vireo::CommandList& commandList) const {
        compute(commandList, opaquePipelinesData);
        compute(commandList, transparentPipelinesData);
    }

    void Scene::compute(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) const {
        for (const auto& [pipelineId, pipelineData] : pipelinesData) {
            const auto& cullingBuffer = *pipelineData->culledDrawCommandsBuffer;
            pipelineData->frustumCullingPipeline.dispatch(
                commandList,
                pipelineData->drawCommandsCount,
                *currentCamera,
                *pipelineData->instancesArray.getBuffer(),
                *pipelineData->drawCommandsBuffer,
                cullingBuffer,
                *pipelineData->culledDrawCommandsCountBuffer);
        }
    }

    void Scene::updatePipelineData(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) {
        for (const auto& [pipelineId, pipelineData] : pipelinesData) {
            if (pipelineData->instancesUpdated) {
                pipelineData->instancesArray.flush(commandList);
                commandList.upload(pipelineData->drawCommandsBuffer, pipelineData->drawCommands.data());
                pipelineData->instancesUpdated = false;
                commandList.barrier(
                    *pipelineData->instancesArray.getBuffer(),
                    vireo::ResourceState::COPY_DST,
                    vireo::ResourceState::SHADER_READ);
                commandList.barrier(
                    *pipelineData->drawCommandsBuffer,
                    vireo::ResourceState::COPY_DST,
                    vireo::ResourceState::SHADER_READ);
                commandList.barrier(
                    *pipelineData->culledDrawCommandsBuffer,
                    vireo::ResourceState::COPY_DST,
                    vireo::ResourceState::INDIRECT_DRAW);
            }
        }
    }

    void Scene::update(vireo::CommandList& commandList) {
        meshInstancesDataArray.restart();

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

        for (const auto& meshInstance : std::views::keys(meshInstancesDataMemoryBlocks)) {
            if (meshInstance->isUpdated()) {
                const auto modelData = meshInstance->getModelData();
                meshInstancesDataArray.write(meshInstancesDataMemoryBlocks[meshInstance], &modelData);
                meshInstancesDataUpdated = true;
                meshInstance->decrementUpdates();
            }
        }

        for (const auto& [pipelineId, materials] : pipelineIds) {
            for (const auto& material : materials) {
                if (material->isUpdated()) {
                    material->upload();
                    Application::getResources().setUpdated();
                    material->decrementUpdates();
                }
            }
        }

        if (meshInstancesDataUpdated) {
            meshInstancesDataArray.flush(commandList);
            meshInstancesDataUpdated = false;
            commandList.barrier(
                *meshInstancesDataArray.getBuffer(),
                vireo::ResourceState::COPY_DST,
                vireo::ResourceState::SHADER_READ);
        }

        updatePipelineData(commandList, opaquePipelinesData);
        updatePipelineData(commandList, transparentPipelinesData);

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
            node->setMaxUpdates(framesInFlight);
            activateCamera(static_pointer_cast<Camera>(node));
            break;
        case Node::ENVIRONMENT:
            node->setMaxUpdates(framesInFlight);
            currentEnvironment = static_pointer_cast<Environment>(node);
            break;
        case Node::MESH_INSTANCE:{
            const auto& meshInstance = static_pointer_cast<MeshInstance>(node);
            if (meshInstancesDataMemoryBlocks.contains(meshInstance)) {
                break;
            }

            const auto& mesh = meshInstance->getMesh();
            assert([&]{return !mesh->getMaterials().empty(); }, "Models without materials are not supported");
            if (!mesh->isUploaded()) {
                mesh->upload();
                Application::getResources().setUpdated();
            }

            meshInstancesDataMemoryBlocks[meshInstance] = meshInstancesDataArray.alloc(1);
            meshInstance->setMaxUpdates(framesInFlight);
            mesh->setMaxUpdates(framesInFlight);
            if (!meshInstance->isUpdated()) { meshInstance->setUpdated(); }

            auto haveTransparentMaterial{false};
            auto nodePipelineIds = std::set<uint32>{};
            for (const auto& material : mesh->getMaterials()) {
                material->setMaxUpdates(framesInFlight);
                haveTransparentMaterial = material->getTransparency() != Transparency::DISABLED;
                auto id = material->getPipelineId();
                nodePipelineIds.insert(id);
                if (!pipelineIds.contains(id)) {
                    pipelineIds[id].push_back(material);
                    materialsUpdated = true;
                }
            }

            for (const auto& pipelineId : nodePipelineIds) {
                if (haveTransparentMaterial) {
                    addNode(pipelineId, meshInstance, transparentPipelinesData);
                } else {
                    addNode(pipelineId, meshInstance, opaquePipelinesData);
                }
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

    void Scene::addNode(
        pipeline_id pipelineId,
        const std::shared_ptr<MeshInstance>& meshInstance,
        std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) {
        if (!pipelinesData.contains(pipelineId)) {
            pipelinesData[pipelineId] = std::make_unique<PipelineData>(config, pipelineId, meshInstancesDataArray);
        }
        pipelinesData[pipelineId]->addNode(meshInstance, meshInstancesDataMemoryBlocks);
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
            const auto& mesh = meshInstance->getMesh();

            if (!meshInstancesDataMemoryBlocks.contains(meshInstance)) {
                break;
            }
            meshInstancesDataArray.free(meshInstancesDataMemoryBlocks.at(meshInstance));
            meshInstancesDataMemoryBlocks.erase(meshInstance);

            auto haveTransparentMaterial{false};
            auto nodePipelineIds = std::set<uint32>{};
            for (const auto& material : mesh->getMaterials()) {
                auto id = material->getPipelineId();
                haveTransparentMaterial = material->getTransparency() != Transparency::DISABLED;
                nodePipelineIds.insert(id);
            }
            for (const auto& pipelineId : nodePipelineIds) {
                if (haveTransparentMaterial) {
                    transparentPipelinesData[pipelineId]->removeNode(meshInstance, meshInstancesDataMemoryBlocks);
                } else {
                    opaquePipelinesData[pipelineId]->removeNode(meshInstance, meshInstancesDataMemoryBlocks);
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
        drawModels(commandList, pipelines, opaquePipelinesData);
    }

    void Scene::drawTransparentModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const {
        drawModels(commandList, pipelines, transparentPipelinesData);
    }

    void Scene::drawModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines,
        const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) const {
        commandList.setViewport(viewport);
        commandList.setScissors(scissors);
        commandList.bindVertexBuffer(Application::getResources().getVertexArray().getBuffer());
        commandList.bindIndexBuffer(Application::getResources().getIndexArray().getBuffer());
        for (const auto& [pipelineId, pipelineData] : pipelinesData) {
            const auto& pipeline = pipelines.at(pipelineId);
            commandList.bindPipeline(pipeline);
            commandList.bindDescriptors({
                Application::getResources().getDescriptorSet(),
                Application::getResources().getSamplers().getDescriptorSet(),
                descriptorSet,
                pipelineData->descriptorSet
            });
            commandList.drawIndexedIndirectCount(
                pipelineData->culledDrawCommandsBuffer,
                0,
                pipelineData->culledDrawCommandsCountBuffer,
                0,
                pipelineData->drawCommandsCount,
                sizeof(DrawCommand),
                sizeof(uint32));
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

    Scene::PipelineData::PipelineData(
        const SceneConfiguration& config,
        const uint32 pipelineId,
        const DeviceMemoryArray& meshInstancesDataArray) :
        pipelineId{pipelineId},
        config{config},
        frustumCullingPipeline{ meshInstancesDataArray},
        instancesArray{
            Application::getVireo(),
            sizeof(InstanceData),
            config.maxMeshSurfacePerFrame,
            config.maxMeshSurfacePerFrame,
            vireo::BufferType::DEVICE_STORAGE,
            L"Pipeline instances array"},
        drawCommands(config.maxMeshSurfacePerFrame),
        drawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::DEVICE_STORAGE,
            sizeof(DrawCommand) * config.maxMeshSurfacePerFrame,
            1,
            L"Pipeline draw commands")},
        culledDrawCommandsCountBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::READWRITE_STORAGE,
            sizeof(uint32),
            1,
            L"Pipeline draw commands counter")},
        culledDrawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::READWRITE_STORAGE,
            sizeof(DrawCommand) * config.maxMeshSurfacePerFrame,
            1,
            L"Pipeline culled draw commands")}
    {
        descriptorSet = Application::getVireo().createDescriptorSet(pipelineDescriptorLayout, L"Pipeline");
        descriptorSet->update(BINDING_INSTANCES, instancesArray.getBuffer());
    }

    void Scene::PipelineData::addNode(
        const std::shared_ptr<MeshInstance>& meshInstance,
        const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& meshInstancesDataMemoryBlocks) {
        const auto& mesh = meshInstance->getMesh();
        const auto instanceMemoryBlock = instancesArray.alloc(mesh->getSurfaces().size());
        instancesMemoryBlocks[meshInstance] = instanceMemoryBlock;
        addInstance(mesh, instanceMemoryBlock, meshInstancesDataMemoryBlocks.at(meshInstance));
    }

    void Scene::PipelineData::addInstance(
        const std::shared_ptr<Mesh>& mesh,
        const MemoryBlock& instanceMemoryBlock,
        const MemoryBlock& meshInstanceMemoryBlock) {
        auto instancesData = std::vector<InstanceData>{};
        for (uint32 i = 0; i < mesh->getSurfaces().size(); i++) {
            const auto& surface = mesh->getSurfaces()[i];
            if (surface->material->getPipelineId() == pipelineId) {
                const uint32 id = instanceMemoryBlock.instanceIndex + instancesData.size();
                drawCommands[drawCommandsCount] = {
                    .instanceIndex = id,
                    .command = {
                        .indexCount = surface->indexCount,
                        .instanceCount = 1,
                        .firstIndex = mesh->getIndicesIndex() + surface->firstIndex,
                        .vertexOffset = static_cast<int32>(mesh->getVerticesIndex()),
                        .firstInstance = id,
                    }
                };
                instancesData.push_back(InstanceData {
                    .meshInstanceIndex = meshInstanceMemoryBlock.instanceIndex,
                    .meshSurfaceIndex = mesh->getSurfacesIndex() + i,
                });
                drawCommandsCount++;
            }
        }
        instancesArray.write(instanceMemoryBlock, instancesData.data());
        instancesUpdated = true;
    }

    void Scene::PipelineData::removeNode(
        const std::shared_ptr<MeshInstance>& meshInstance,
        const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& meshInstancesDataMemoryBlocks) {
        if (instancesMemoryBlocks.contains(meshInstance)) {
            instancesArray.free(instancesMemoryBlocks.at(meshInstance));
            instancesMemoryBlocks.erase(meshInstance);
        }
        drawCommandsCount = 0;
        for (const auto& instance : std::views::keys(instancesMemoryBlocks)) {
            addInstance(
                instance->getMesh(),
                instancesMemoryBlocks.at(instance),
                meshInstancesDataMemoryBlocks.at(instance));
        }
    }


}
