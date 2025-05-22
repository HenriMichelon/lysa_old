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

    Scene::Scene(const RenderingConfiguration& config, const vireo::Extent &extent) :
        config{config},
        sceneUniformBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneData), 1,
            L"Scene Data")},
        instancesDataArray{Application::getVireo(),
            sizeof(MeshSurfaceInstanceData),
            1000, // TODO make dynamic
            1000, // TODO make dynamic
            vireo::BufferType::STORAGE,
            L"MeshSurface Instance Data"},
        instancesIndexArray{Application::getVireo(),
            sizeof(Index),
            1000, // TODO make dynamic
            1000, // TODO make dynamic
            vireo::BufferType::STORAGE,
            L"Draw indices"},
        opaqueDrawCommandsBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::INDIRECT,
            sizeof(vireo::DrawIndexedIndirectCommand), 1,
            L"Draw commands")},
        opaqueDrawCommandsStagingBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::BUFFER_UPLOAD,
            sizeof(vireo::DrawIndexedIndirectCommand), 1,
        L"Draw commands upload")} {
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
        descriptorSet->update(BINDING_INSTANCE_INDEX, instancesIndexArray.getBuffer());
        sceneUniformBuffer->map();
        opaqueDrawCommandsStagingBuffer->map();
        resize(extent);
    }

    void Scene::resize(const vireo::Extent& extent) {
        this->extent = extent;
        if (viewportAndScissors == nullptr) {
            viewport = vireo::Viewport{
                .width = static_cast<float>(extent.width),
                .height = static_cast<float>(extent.height)
            };
            scissors = vireo::Rect{
                .width = extent.width,
                .height = extent.height
            };
        }
    }

    void Scene::update(vireo::CommandList& commandList) {
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
        if (resourcesUpdated) {
            Application::getResources().flush(commandList);
            resourcesUpdated = false;
        }

        if (instancesDataUpdated) {
            instancesDataArray.flush(commandList);
            instancesDataUpdated = false;
        }
        if (instancesIndexUpdated) {
            auto instancesIndex = std::vector<Index>{};
            for (const auto& meshInstance : models) {
                const auto& memoryBloc = instancesDataMemoryBlocks[meshInstance];
                const auto& mesh = meshInstance->getMesh();
                const auto& surfaces = mesh->getSurfaces();
                auto& meshIndices = mesh->getIndices();
                for (int surfaceIndex = 0; surfaceIndex < surfaces.size(); surfaceIndex++) {
                    const auto instanceDataIndex = memoryBloc.instanceIndex + surfaceIndex;
                    for (int i = 0; i < surfaces[surfaceIndex]->indexCount; i++) {
                        instancesIndex.push_back({
                            .index = meshIndices[surfaces[surfaceIndex]->firstIndex + i],
                            .surfaceIndex = instanceDataIndex
                        });
                    }
                }
            }
            if (instancesIndex.size() > 0) {
                const auto instancesIndexMemoryBlock = instancesIndexArray.alloc(instancesIndex.size());
                instancesIndexArray.write(instancesIndexMemoryBlock, instancesIndex.data());
                instancesIndexArray.flush(commandList);
                instancesIndexArray.free(instancesIndexMemoryBlock);
                const auto drawCommand = vireo::DrawIndirectCommand {
                    .vertexCount = static_cast<uint32>(instancesIndex.size())
                };
                opaqueDrawCommandsStagingBuffer->write(&drawCommand, sizeof(drawCommand));
                commandList.copy(opaqueDrawCommandsStagingBuffer, opaqueDrawCommandsBuffer);
            }
            instancesIndexUpdated = false;
        }
    }

    void Scene::addNode(const std::shared_ptr<Node>& node) {
        switch (node->getType()) {
        case Node::CAMERA:
            activateCamera(static_pointer_cast<Camera>(node));
            break;
        case Node::MESH_INSTANCE:{
            const auto& meshInstance = static_pointer_cast<MeshInstance>(node);
            const auto& mesh = meshInstance->getMesh();
            assert([&]{return !mesh->getMaterials().empty(); }, "Models without materials are not supported");
            if (!mesh->isUploaded()) {
                mesh->upload();
                resourcesUpdated = true;
            }
            instancesIndexUpdated = true;
            models.push_back(meshInstance);
            opaqueModels.push_back(meshInstance);
            // Force model data to be written to GPU memory
            meshInstance->updated = config.framesInFlight;

            const auto& surfaces = mesh->getSurfaces();
            instancesDataMemoryBlocks[meshInstance] = instancesDataArray.alloc(surfaces.size());
            auto instancesData = std::vector<MeshSurfaceInstanceData>{surfaces.size()};
            for (int i = 0; i < surfaces.size(); i++) {
                instancesData[i].transform = meshInstance->getTransformGlobal();
                instancesData[i].vertexIndex = mesh->getVertexIndex();
                instancesData[i].materialIndex = surfaces[i]->material->getMaterialIndex();
            }
            instancesDataArray.write(instancesDataMemoryBlocks[meshInstance], instancesData.data());
            instancesDataUpdated = true;
            break;
        }
        case Node::VIEWPORT:
            viewportAndScissors = static_pointer_cast<Viewport>(node);
            viewport = viewportAndScissors->getViewport();
            scissors = viewportAndScissors->getScissors();
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
        case Node::MESH_INSTANCE:{
            const auto& meshInstance = static_pointer_cast<MeshInstance>(node);
            break;
        }
        case Node::VIEWPORT:
            if (node == viewportAndScissors) {
                viewportAndScissors.reset();
                resize(extent);
            }
            break;
        default:
            break;
        }
    }

    void Scene::draw(
        vireo::CommandList& commandList,
        const vireo::Pipeline& pipeline,
        const Samplers& samplers) const {
        auto& resources = Application::getResources();
        const auto sets = std::vector<std::shared_ptr<const vireo::DescriptorSet>> {
            resources.getDescriptorSet(),
            descriptorSet,
            samplers.getDescriptorSet()};
        commandList.setDescriptors(sets);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors(pipeline, sets);
        commandList.drawIndirect(opaqueDrawCommandsBuffer, 0, 1, sizeof(vireo::DrawIndirectCommand));

        // for (const auto& command : commands) {
        // commandList->drawIndexedIndirect(
        // commandBuffer,
        // 0,
        // commands.size(),
        // sizeof(vireo::DrawIndexedIndirectCommand));
        // }
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
}
