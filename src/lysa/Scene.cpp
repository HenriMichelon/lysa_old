/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module;
#include <cstddef>
module lysa.scene;

import vireo;
import lysa.application;
import lysa.log;

namespace lysa {

    const std::vector<vireo::VertexAttributeDesc> Scene::vertexAttributes{
        {"INDEX",   vireo::AttributeFormat::R32_FLOAT, offsetof(IndexData, index)},
        {"MODEL",   vireo::AttributeFormat::R32_FLOAT, offsetof(IndexData, modelIndex)},
        {"SURFACE", vireo::AttributeFormat::R32_FLOAT, offsetof(IndexData, surfaceIndex)},
    };

    void Scene::createDescriptorLayouts() {
        sceneDescriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
        sceneDescriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
        sceneDescriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::DEVICE_STORAGE);
        sceneDescriptorLayout->add(BINDING_LIGHTS, vireo::DescriptorType::UNIFORM);
        sceneDescriptorLayout->build();
    }

    void Scene::destroyDescriptorLayouts() {
        sceneDescriptorLayout.reset();
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
        // frustumCullingPipeline{ modelsDataArray,surfacesDataArray },
        modelsDataArray{Application::getVireo(),
            sizeof(ModelData),
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
        descriptorSet->update(BINDING_MODELS, modelsDataArray.getBuffer());
        descriptorSet->update(BINDING_LIGHTS, lightsBuffer);
        sceneUniformBuffer->map();
        lightsBuffer->map();

        opaquePipelinesData[DEFAULT_PIPELINE_ID] = std::make_unique<PipelineData>(PipelineData{config, DEFAULT_PIPELINE_ID});
    }

    void Scene::compute(vireo::CommandList& commandList) {
        // for (const auto& [pipelineId, pipelineData] : opaquePipelinesData) {
        //     const auto& cullingBuffer = *pipelineData->instancesIndexBuffer;
        //     commandList.barrier(
        //         cullingBuffer,
        //         vireo::ResourceState::SHADER_READ,
        //         vireo::ResourceState::COMPUTE_WRITE);
        //     frustumCullingPipeline.dispatch(
        //         commandList,
        //         pipelineId,
        //         surfaceCount,
        //         *currentCamera,
        //         cullingBuffer,
        //         *pipelineData->drawCommandsBuffer);
        //     commandList.barrier(
        //         cullingBuffer,
        //         vireo::ResourceState::COMPUTE_WRITE,
        //         vireo::ResourceState::SHADER_READ);
        // }
    }

    void Scene::update(const vireo::CommandList& commandList) {
        modelsDataArray.restart();

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
            if (meshInstance->isUpdated()) {
                const auto modelData = meshInstance->getModelData();
                modelsDataArray.write(modelsDataMemoryBlocks[meshInstance], &modelData);
                modelsDataUpdated = true;
                meshInstance->decrementUpdates();

                const auto& mesh = meshInstance->getMesh();
                for (const auto& material : mesh->getMaterials()) {
                    if (material->isUpdated()) {
                        material->upload();
                        Application::getResources().setUpdated();
                        material->decrementUpdates();
                    }
                }
            }
        }

        if (modelsDataUpdated) {
            modelsDataArray.flush(commandList);
            modelsDataUpdated = false;
            commandList.barrier(
                *modelsDataArray.getBuffer(),
                vireo::ResourceState::COPY_DST,
                vireo::ResourceState::SHADER_READ);
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
            node->setMaxUpdates(framesInFlight);
            activateCamera(static_pointer_cast<Camera>(node));
            break;
        case Node::ENVIRONMENT:
            node->setMaxUpdates(framesInFlight);
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
            modelsDataMemoryBlocks[meshInstance] = modelsDataArray.alloc(1);
            meshInstance->setMaxUpdates(framesInFlight);
            mesh->setMaxUpdates(framesInFlight);
            if (!meshInstance->isUpdated()) { meshInstance->setUpdated(); }

            auto nodePipelineIds = std::list<uint32>{};
            for (const auto& material : mesh->getMaterials()) {
                material->setMaxUpdates(framesInFlight);
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
                opaquePipelinesData[pipelineId]->addNode(meshInstance);
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
            models.remove(meshInstance);
            if (modelsDataMemoryBlocks.contains(meshInstance)) {
                modelsDataArray.free(modelsDataMemoryBlocks.at(meshInstance));
                modelsDataMemoryBlocks.erase(meshInstance);
            }

            const auto& mesh = meshInstance->getMesh();
            auto nodePipelineIds = std::list<uint32>{};
            for (const auto& material : mesh->getMaterials()) {
                auto id = material->getPipelineId();
                nodePipelineIds.push_back(id);
            }
            for (const auto& pipelineId : nodePipelineIds) {
                opaquePipelinesData[pipelineId]->removeNode(meshInstance);
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
            const auto& pipeline = pipelines.at(pipelineId);
            commandList.bindPipeline(pipeline);
            commandList.bindDescriptors(pipeline, {
                Application::getResources().getDescriptorSet(),
                Application::getResources().getSamplers().getDescriptorSet(),
                descriptorSet
            });
            // commandList.drawIndirect(pipelineData->drawCommandsBuffer, 0, 1, sizeof(vireo::DrawIndirectCommand));
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

    Scene::PipelineData::PipelineData(const SceneConfiguration& config, const uint32 pipelineId) :
        pipelineId{pipelineId},
        config{config},
        drawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::INDIRECT,
            sizeof(vireo::DrawIndexedIndirectCommand), 1,
            L"Draw commands")},
        drawDataBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::VERTEX,
            sizeof(IndexData) * config.maxVertexPerFramePerPipeline, 1,
            L"Draw data")}
    {
    }

    void Scene::PipelineData::addNode(
        const std::shared_ptr<MeshInstance>& meshInstance) {
        models.push_back(meshInstance);
    }

    void Scene::PipelineData::removeNode(
        const std::shared_ptr<MeshInstance>& meshInstance) {
        models.remove(meshInstance);
    }


}
