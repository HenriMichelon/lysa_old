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
        config{config},
        sceneUniformBuffer{Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneData), 1,
            L"Scene Data")} {
        if (descriptorLayout == nullptr) {
            descriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
            descriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
            descriptorLayout->build();
        }

        descriptorSet = Application::getVireo().createDescriptorSet(descriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);

        resize(extent);
        sceneUniformBuffer->map();
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

    void Scene::update() const {
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
        const std::shared_ptr<vireo::CommandList>& commandList,
        const std::shared_ptr<vireo::Pipeline>& pipeline,
        const Samplers& samplers,
        const std::vector<vireo::DrawIndexedIndirectCommand>& commands,
        const std::shared_ptr<vireo::Buffer>& commandBuffer) const {
        auto& resources = Application::getResources();
        const auto sets = std::vector<std::shared_ptr<const vireo::DescriptorSet>> {
            resources.getDescriptorSet(),
            getDescriptorSet(),
            samplers.getDescriptorSet()};
        commandList->setDescriptors(sets);
        commandList->bindPipeline(pipeline);
        commandList->bindDescriptors(pipeline, sets);
        //commandList->bindIndexBuffer(XXX);
        for (const auto& command : commands) {
            commandList->drawIndexedIndirect(
                commandBuffer,
                0,
                commands.size(),
                sizeof(vireo::DrawIndexedIndirectCommand));
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
