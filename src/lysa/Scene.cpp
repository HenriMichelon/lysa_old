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
import lysa.viewport;
import lysa.window;
import lysa.renderers.renderpass.shadow_map_pass;

namespace lysa {

    void Scene::createDescriptorLayouts() {
        sceneDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
        sceneDescriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
        sceneDescriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::DEVICE_STORAGE);
        sceneDescriptorLayout->add(BINDING_LIGHTS, vireo::DescriptorType::UNIFORM);
        sceneDescriptorLayout->add(BINDING_SHADOW_MAPS, vireo::DescriptorType::SAMPLED_IMAGE, MAX_SHADOW_MAPS * 6);
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
        const RenderingConfiguration& renderingConfig,
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
            config.maxModelsPerScene,
            config.maxModelsPerScene,
            vireo::BufferType::DEVICE_STORAGE,
            L"Models Data"},
        sceneUniformBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneData), 1,
            L"Scene Data")},
        scissors{scissors},
        viewport{viewport},
        framesInFlight{framesInFlight},
        renderingConfig{renderingConfig} {

        shadowMaps.resize(MAX_SHADOW_MAPS * 6);
        for (int i = 0; i < shadowMaps.size(); i++) {
            shadowMaps[i] = Application::getResources().getBlankImage();
        }

        descriptorSet = Application::getVireo().createDescriptorSet(sceneDescriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
        descriptorSet->update(BINDING_MODELS, meshInstancesDataArray.getBuffer());
        descriptorSet->update(BINDING_LIGHTS, lightsBuffer);
        descriptorSet->update(BINDING_SHADOW_MAPS, shadowMaps);

        sceneUniformBuffer->map();
        lightsBuffer->map();
    }

    void Scene::compute(vireo::CommandList& commandList) const {
        compute(commandList, opaquePipelinesData);
        compute(commandList, shaderMaterialPipelinesData);
        compute(commandList, transparentPipelinesData);
    }

    void Scene::compute(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) const {
        for (const auto& [pipelineId, pipelineData] : pipelinesData) {
            pipelineData->frustumCullingPipeline.dispatch(
                commandList,
                pipelineData->drawCommandsCount,
                currentCamera->getTransformGlobal(),
                currentCamera->getProjection(),
                *pipelineData->instancesArray.getBuffer(),
                *pipelineData->drawCommandsBuffer,
                *pipelineData->culledDrawCommandsBuffer,
                *pipelineData->culledDrawCommandsCountBuffer);
            for (const auto& renderer : std::views::values(shadowMapRenderers)) {
                const auto& shadowMapRenderer = std::static_pointer_cast<ShadowMapPass>(renderer);
                shadowMapRenderer->compute(commandList, pipelinesData);
            }
        }
    }

    void Scene::updatePipelinesData(
        const vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) {
        for (const auto& [pipelineId, pipelineData] : pipelinesData) {
            pipelineData->updateData(commandList, drawCommandsStagingBufferRecycleBin);
        }
    }

    void Scene::update(const vireo::CommandList& commandList) {
        if (!drawCommandsStagingBufferRecycleBin.empty()) {
            drawCommandsStagingBufferRecycleBin.clear();
        }
        meshInstancesDataArray.restart();

        if (shadowMapsUpdated) {
            descriptorSet->update(BINDING_SHADOW_MAPS, shadowMaps);
            shadowMapsUpdated = false;
        }

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

        if (meshInstancesDataUpdated) {
            meshInstancesDataArray.flush(commandList);
            meshInstancesDataUpdated = false;
        }

        updatePipelinesData(commandList, opaquePipelinesData);
        updatePipelinesData(commandList, shaderMaterialPipelinesData);
        updatePipelinesData(commandList, transparentPipelinesData);

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
                if (light->isVisible()) {
                    lightsArray[lightIndex] = light->getLightData();
                    if (shadowMapRenderers.contains(light)) {
                        const auto&shadowMapRenderer = std::static_pointer_cast<ShadowMapPass>(shadowMapRenderers[light]);
                        lightsArray[lightIndex].mapIndex = shadowMapIndex[light];
                        switch (light->getLightType()) {
                            case Light::LIGHT_DIRECTIONAL: {
                                // lightsArray[lightIndex].cascadesCount = shadowMapRenderer->getCascadesCount(currentFrame);
                                // for (int cascadeIndex = 0; cascadeIndex < lightsArray[lightIndex].cascadesCount ; cascadeIndex++) {
                                    // lightsArray[lightIndex].lightSpace[cascadeIndex] = shadowMapRenderer->getLightSpace(cascadeIndex, currentFrame);
                                    // lightsArray[lightIndex].cascadeSplitDepth[cascadeIndex] = shadowMapRenderer->getCascadeSplitDepth(cascadeIndex, currentFrame);
                                // }
                                break;
                            }
                            case Light::LIGHT_SPOT: {
                                lightsArray[lightIndex].lightSpace[0] = shadowMapRenderer->getLightSpace(0);
                                break;
                            }
                            case Light::LIGHT_OMNI: {
                                // lightsArray[lightIndex].farPlane = shadowMapRenderer->getFarPlane();
                                // for (int faceIndex = 0; faceIndex < 6; faceIndex++) {
                                    // lightsArray[lightIndex].lightSpace[faceIndex] =
                                            // shadowMapRenderer->getLightSpace(faceIndex, currentFrame);
                                // }
                                break;
                            }
                            default:;
                        }
                    }
                    lightIndex++;
                }
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
            if (!meshInstance->isUpdated()) { meshInstance->setUpdated(); }

            auto haveTransparentMaterial{false};
            auto haveShaderMaterial{false};
            auto nodePipelineIds = std::set<uint32>{};
            for (int i = 0; i < mesh->getSurfaces().size(); i++) {
                const auto material = meshInstance->getSurfaceMaterial(i);
                haveTransparentMaterial = material->getTransparency() != Transparency::DISABLED;
                haveShaderMaterial = material->getType() == Material::SHADER;
                auto id = material->getPipelineId();
                nodePipelineIds.insert(id);
                if (!pipelineIds.contains(id)) {
                    pipelineIds[id].push_back(material);
                    materialsUpdated = true;
                }
            }

            for (const auto& pipelineId : nodePipelineIds) {
                if (haveShaderMaterial) {
                    addNode(pipelineId, meshInstance, shaderMaterialPipelinesData);
                } else if (haveTransparentMaterial) {
                    addNode(pipelineId, meshInstance, transparentPipelinesData);
                } else {
                    addNode(pipelineId, meshInstance, opaquePipelinesData);
                }
            }
            break;
        }
        case Node::DIRECTIONAL_LIGHT:
        case Node::OMNI_LIGHT:
        case Node::SPOT_LIGHT:
            lights.push_back(static_pointer_cast<Light>(node));
            enableLightShadowCasting(node);
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
            if (!meshInstancesDataMemoryBlocks.contains(meshInstance)) {
                break;
            }
            for (const auto& pipelineId : std::views::keys(pipelineIds)) {
                if (shaderMaterialPipelinesData.contains(pipelineId)) {
                    shaderMaterialPipelinesData[pipelineId]->removeNode(meshInstance, meshInstancesDataMemoryBlocks);
                }
                if (transparentPipelinesData.contains(pipelineId)) {
                    transparentPipelinesData[pipelineId]->removeNode(meshInstance, meshInstancesDataMemoryBlocks);
                }
                if (opaquePipelinesData.contains(pipelineId)) {
                    opaquePipelinesData[pipelineId]->removeNode(meshInstance, meshInstancesDataMemoryBlocks);
                }
            }
            meshInstancesDataArray.free(meshInstancesDataMemoryBlocks.at(meshInstance));
            meshInstancesDataMemoryBlocks.erase(meshInstance);
            break;
        }
        case Node::DIRECTIONAL_LIGHT:
        case Node::OMNI_LIGHT:
        case Node::SPOT_LIGHT:
            lights.remove(static_pointer_cast<Light>(node));
            // node->getViewport()->getWindow().disableLightShadowCasting(node);
            break;
        default:
            break;
        }
    }

    void Scene::drawOpaquesModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const {
        if (opaquePipelinesData.empty()) { return; }
        drawModels(commandList, pipelines, opaquePipelinesData);
    }

    void Scene::drawTransparentModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const {
        if (transparentPipelinesData.empty()) { return; }
        drawModels(commandList, pipelines, transparentPipelinesData);
    }

    void Scene::drawShaderMaterialModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const {
        if (shaderMaterialPipelinesData.empty()) { return; }
        drawModels(commandList, pipelines, shaderMaterialPipelinesData);
    }

    void Scene::setInitialState(const vireo::CommandList& commandList) const {
        commandList.setViewport(viewport);
        commandList.setScissors(scissors);
    }

    void Scene::drawOpaquesAndShaderMaterialsModels(
        vireo::CommandList& commandList,
        const uint32 set,
        const std::map<pipeline_id, std::shared_ptr<vireo::Buffer>>& culledDrawCommandsBuffers,
        const std::map<pipeline_id, std::shared_ptr<vireo::Buffer>>& culledDrawCommandsCountBuffers) const {
        for (const auto& [pipelineId, pipelineData] : opaquePipelinesData) {
            commandList.bindDescriptor(pipelineData->descriptorSet, set);
            // commandList.drawIndexedIndirect(
            //     pipelineData->drawCommandsBuffer,
            //     0,
            //     pipelineData->drawCommandsCount,
            //     sizeof(DrawCommand),
            //     sizeof(uint32));
            commandList.drawIndexedIndirectCount(
                culledDrawCommandsBuffers.at(pipelineId),
                0,
                culledDrawCommandsCountBuffers.at(pipelineId),
                0,
                pipelineData->drawCommandsCount,
                sizeof(DrawCommand),
                sizeof(uint32));
        }
        for (const auto& [pipelineId, pipelineData] : shaderMaterialPipelinesData) {
            commandList.bindDescriptor(pipelineData->descriptorSet, set);
            commandList.drawIndexedIndirectCount(
                culledDrawCommandsBuffers.at(pipelineId),
                0,
                culledDrawCommandsCountBuffers.at(pipelineId),
                0,
                pipelineData->drawCommandsCount,
                sizeof(DrawCommand),
                sizeof(uint32));
        }
    }

    void Scene::drawModels(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines,
        const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) const {
        for (const auto& [pipelineId, pipelineData] : pipelinesData) {
            if (pipelineData->drawCommandsCount == 0) { continue; }
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
        frustumCullingPipeline{true, meshInstancesDataArray},
        instancesArray{
            Application::getVireo(),
            sizeof(InstanceData),
            config.maxMeshSurfacePerPipeline,
            config.maxMeshSurfacePerPipeline,
            vireo::BufferType::DEVICE_STORAGE,
            L"Pipeline instances array"},
        drawCommands(config.maxMeshSurfacePerPipeline),
        drawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::DEVICE_STORAGE,
            sizeof(DrawCommand) * config.maxMeshSurfacePerPipeline,
            1,
            L"Pipeline draw commands")},
        culledDrawCommandsCountBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::READWRITE_STORAGE,
            sizeof(uint32),
            1,
            L"Pipeline draw commands counter")},
        culledDrawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::READWRITE_STORAGE,
            sizeof(DrawCommand) * config.maxMeshSurfacePerPipeline,
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
        addInstance(meshInstance, instanceMemoryBlock, meshInstancesDataMemoryBlocks.at(meshInstance));
    }

    void Scene::PipelineData::addInstance(
        const std::shared_ptr<MeshInstance>& meshInstance,
        const MemoryBlock& instanceMemoryBlock,
        const MemoryBlock& meshInstanceMemoryBlock) {
        const auto& mesh = meshInstance->getMesh();
        auto instancesData = std::vector<InstanceData>{};
        for (uint32 i = 0; i < mesh->getSurfaces().size(); i++) {
            const auto& surface = mesh->getSurfaces()[i];
            const auto& material = meshInstance->getSurfaceMaterial(i);
            if (material->getPipelineId() == pipelineId) {
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
                    .materialIndex = material->getMaterialIndex(),
                    .meshSurfaceMaterialIndex = mesh->getSurfaceMaterial(i)->getMaterialIndex(),
                });
                drawCommandsCount++;
            }
        }
        if (!instancesData.empty()) {
            instancesArray.write(instanceMemoryBlock, instancesData.data());
            instancesUpdated = true;
        }
    }

    void Scene::PipelineData::removeNode(
        const std::shared_ptr<MeshInstance>& meshInstance,
        const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& meshInstancesDataMemoryBlocks) {
        if (instancesMemoryBlocks.contains(meshInstance)) {
            instancesArray.free(instancesMemoryBlocks.at(meshInstance));
            instancesMemoryBlocks.erase(meshInstance);
            drawCommandsCount = 0;
            for (const auto& instance : std::views::keys(instancesMemoryBlocks)) {
                addInstance(
                    instance,
                    instancesMemoryBlocks.at(instance),
                    meshInstancesDataMemoryBlocks.at(instance));
            }
        }
    }

    void Scene::PipelineData::updateData(
        const vireo::CommandList& commandList,
        std::unordered_set<std::shared_ptr<vireo::Buffer>>& drawCommandsStagingBufferRecycleBin) {
        if (instancesUpdated) {
            instancesArray.flush(commandList);
            if (drawCommandsStagingBufferCount < drawCommandsCount) {
                if (drawCommandsStagingBuffer) {
                    drawCommandsStagingBufferRecycleBin.insert(drawCommandsStagingBuffer);
                }
                drawCommandsStagingBuffer = Application::getVireo().createBuffer(
                    vireo::BufferType::BUFFER_UPLOAD,
                    sizeof(DrawCommand) * drawCommandsCount);
                drawCommandsStagingBufferCount = drawCommandsCount;
                drawCommandsStagingBuffer->map();
            }

            drawCommandsStagingBuffer->write(drawCommands.data(),
                sizeof(DrawCommand) * drawCommandsCount);
            commandList.copy(drawCommandsStagingBuffer, drawCommandsBuffer, sizeof(DrawCommand) * drawCommandsCount);
            instancesUpdated = false;
            commandList.barrier(
                *drawCommandsBuffer,
                vireo::ResourceState::COPY_DST,
                vireo::ResourceState::INDIRECT_DRAW);
            commandList.barrier(
                *culledDrawCommandsBuffer,
                vireo::ResourceState::COPY_DST,
                vireo::ResourceState::INDIRECT_DRAW);
        }
    }

    void Scene::enableLightShadowCasting(const std::shared_ptr<Node>&node) {
        if (const auto& light = std::dynamic_pointer_cast<Light>(node)) {
            if (light->getCastShadows() && !shadowMapRenderers.contains(light) && (shadowMapRenderers.size() < MAX_SHADOW_MAPS)) {
                const auto shadowMapRenderer = make_shared<ShadowMapPass>(
                    config,
                    renderingConfig,
                    light,
                    meshInstancesDataArray);
                shadowMapRenderers[light] = shadowMapRenderer;
                const auto& blankImage = Application::getResources().getBlankImage();
                for (uint32 index = 0; index < shadowMaps.size(); index += 6) {
                    if (shadowMaps[index] == blankImage) {
                        shadowMapIndex[light] = index;
                        for (int i = 0; i < shadowMapRenderer->getShadowMapCount(); i++) {
                            shadowMaps[index + i] = shadowMapRenderer->getShadowMap(i)->getImage();
                        }
                        shadowMapsUpdated = true;
                        return;
                    }
                }
                throw Exception("Out of memory for shadow map");
            }
        }
    }

    void Scene::disableLightShadowCasting(const std::shared_ptr<Node>&node) {
        throw Exception("not implemented");
    }

}
