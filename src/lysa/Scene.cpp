/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.scene;

namespace lysa {

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

    void Scene::addNode(const std::shared_ptr<Node>& node) {
        switch (node->getType()) {
        case Node::CAMERA:
            activateCamera(static_pointer_cast<Camera>(node));
            break;
        case Node::MESH_INSTANCE:{
            const auto& meshInstance = static_pointer_cast<MeshInstance>(node);
            assert([&]{return !meshInstance->getMesh()->getMaterials().empty(); }, "Models without materials are not supported");
            models.push_back(meshInstance);
            modelsUpdated = true;
            for (const auto &material : meshInstance->getMesh()->getMaterials()) {
                if (materialsRefCounter.contains(material->getId())) {
                    materialsRefCounter[material->getId()]++;
                    continue;
                }
                materialsIndices[material->getId()] = static_cast<int32_t>(materials.size());
                materials.push_back(material);
                materialsRefCounter[material->getId()]++;
                // Force material data to be written to GPU memory
                material->updated = framesInFlight;
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
            const auto it = std::ranges::find(models, meshInstance);
            if (it != models.end()) {
                models.erase(it);
                modelsUpdated = true;
                for (const auto &material : meshInstance->getMesh()->getMaterials()) {
                    if (materialsRefCounter.contains(material->getId())) {
                        // Check if we need to remove the material from the scene
                        if (--materialsRefCounter[material->getId()] == 0) {
                            materialsRefCounter.erase(material->getId());
                            // Try to remove the associated textures
                            //...
                            // Remove the material from the scene
                            materials.remove(material);
                            // Rebuild the material index
                            materialsIndices.clear();
                            uint32_t materialIndex = 0;
                            for (const auto &mat : materials) {
                                materialsIndices[mat->getId()] = static_cast<int32_t>(materialIndex);
                                materialIndex += 1;
                            }
                            materialsUpdated = true;
                        }
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
