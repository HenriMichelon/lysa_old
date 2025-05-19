/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.scene;

namespace lysa {

    Scene::Scene(const RenderingConfiguration& config, const std::shared_ptr<vireo::Vireo>& vireo, const vireo::Extent &extent) :
        config{config}, vireo{vireo} {
        materials.resize(config.memoryConfig.maxMaterialCount);
        resize(extent);
        commandAllocator = vireo->createCommandAllocator(vireo::CommandType::TRANSFER);
        transferQueue = vireo->createSubmitQueue(vireo::CommandType::TRANSFER);
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

    // bool Scene::updateModel(const std::shared_ptr<MeshInstance>& meshInstance) {
    //     for (int i = lastModelIndex; i < models.size(); i++) {
    //         if (models[i] == nullptr) {
    //             models[i] = meshInstance;
    //             modelsIndices[meshInstance->getId()] = i;
    //             lastModelIndex = i + 1;
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    bool Scene::updateMaterial(const std::shared_ptr<Material>& material) {
        for (int i = lastMaterialIndex; i < materials.size(); i++) {
            if (materials[i] == nullptr) {
                materials[i] = material;
                materialsIndices[material->getId()] = i;
                lastMaterialIndex = i + 1;
                return true;
            }
        }
        return false;
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
            // Force model data to be written to GPU memory
            meshInstance->updated = config.framesInFlight;

            const auto pair = BufferPair{mesh->getVertexBuffer(), mesh->getIndexBuffer()};
            if (!opaqueModels.contains(pair)) {
                opaqueModels[pair] = {};
            }
            opaqueModels[pair].push_back(meshInstance);

            for (const auto& meshSurface : mesh->getSurfaces()) {
                if (!opaqueDrawCommands.contains(pair)) {
                    opaqueDrawCommands[pair] = {};
                    opaqueDrawCommandsBuffer[pair] = vireo->createBuffer(
                        vireo::BufferType::INDIRECT,
                        sizeof(vireo::DrawIndexedIndirectCommand) * config.memoryConfig.maxMeshSurfacePerBufferCount, 1,
                        L"Per buffer draw commands");
                }
                auto& commands = opaqueDrawCommands[pair];
                commands.push_back(vireo::DrawIndexedIndirectCommand{
                    .indexCount = meshSurface->indexCount,
                    .instanceCount = 1,
                    .firstIndex = mesh->getFirstIndex() + meshSurface->firstIndex,
                    .vertexOffset = mesh->getVertexOffset(),
                    .firstInstance = 0, //static_cast<uint32_t>(commands.size()),
                });
            }
            // const auto cmdList = commandAllocator->createCommandList();
            // cmdList->begin();
            // cmdList->upload(opaqueDrawCommandsBuffer[pair], opaqueDrawCommands[pair].data());
            // cmdList->end();
            // transferQueue->submit({cmdList});
            // transferQueue->waitIdle();

            for (const auto &material :mesh->getMaterials()) {
                if (materialsRefCounter.contains(material->getId())) {
                    materialsRefCounter[material->getId()]++;
                    continue;
                }
                if (!updateMaterial(material)) {
                    lastMaterialIndex = 0;
                    if (!updateMaterial(material)) {
                        throw Exception{"MemoryConfiguration.maxMaterialsCount reached."};
                    }
                }
                // Force material data to be written to GPU memory
                material->updated = config.framesInFlight;
            }
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
            // Remove the model from the scene
            // models[modelsIndices[meshInstance->getId()]].reset();
            // modelsIndices.erase(meshInstance->getId());
            // Check if we need to remove the material from the scene
            for (const auto &material : meshInstance->getMesh()->getMaterials()) {
                if (materialsRefCounter.contains(material->getId())) {
                    if (--materialsRefCounter[material->getId()] == 0) {
                        materialsRefCounter.erase(material->getId());
                        // Try to remove the associated textures
                        //...
                        // Remove the material from the scene
                        materials[materialsIndices[material->getId()]].reset();
                        materialsIndices.erase(material->getId());
                    }
                }
            }
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
