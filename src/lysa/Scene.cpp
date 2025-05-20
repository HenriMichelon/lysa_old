/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.scene;

import lysa.application;

namespace lysa {

    Scene::Scene(const RenderingConfiguration& config, const vireo::Extent &extent) :
        config{config} {
        // materials.resize(1000);
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
        // for (int i = lastMaterialIndex; i < materials.size(); i++) {
        //     if (materials[i] == nullptr) {
        //         materials[i] = material;
        //         materialsIndices[material->getId()] = i;
        //         lastMaterialIndex = i + 1;
        //         return true;
        //     }
        // }
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
            if (!mesh->isUploaded()) {
                mesh->upload();
                memoryUpdated = true;
            }
            opaqueModels.push_back(meshInstance);
            // Force model data to be written to GPU memory
            meshInstance->updated = config.framesInFlight;

            for (const auto& meshSurface : mesh->getSurfaces()) {
                opaqueDrawCommands.push_back(vireo::DrawIndexedIndirectCommand{
                    .indexCount = meshSurface->indexCount,
                    .firstIndex = mesh->getFirstIndex() + meshSurface->firstIndex,
                    .vertexOffset = static_cast<int32_t>(mesh->getFirstVertex()),
                });
            }
            commandsUpdated = true;

            if (opaqueDrawCommandsBuffer == nullptr) {
                opaqueDrawCommandsBuffer = Application::getVireo().createBuffer(
                    vireo::BufferType::INDIRECT,
                    sizeof(vireo::DrawIndexedIndirectCommand) * 1000, 1, // TODO automatic grows
                L"Draw commands");
            }

            // for (const auto &material :mesh->getMaterials()) {
            //     if (materialsRefCounter.contains(material->getId())) {
            //         materialsRefCounter[material->getId()]++;
            //         continue;
            //     }
            //     if (!updateMaterial(material)) {
            //         lastMaterialIndex = 0;
            //         if (!updateMaterial(material)) {
            //             throw Exception{"MemoryConfiguration.maxMaterialsCount reached."};
            //         }
            //     }
            //     // Force material data to be written to GPU memory
            //     material->updated = config.framesInFlight;
            // }
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
            // for (const auto &material : meshInstance->getMesh()->getMaterials()) {
            //     if (materialsRefCounter.contains(material->getId())) {
            //         if (--materialsRefCounter[material->getId()] == 0) {
            //             materialsRefCounter.erase(material->getId());
            //             // Try to remove the associated textures
            //             //...
            //             // Remove the material from the scene
            //             materials[materialsIndices[material->getId()]].reset();
            //             materialsIndices.erase(material->getId());
            //         }
            //     }
            // }
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
