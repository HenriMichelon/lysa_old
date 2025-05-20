/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.renderers.scene_data;

import lysa.application;
import lysa.nodes.node;
import lysa.resources.mesh;

namespace lysa {

    SceneData::SceneData(const RenderingConfiguration& config, const vireo::Extent &extent):
        Scene{config, extent}  {
        if (descriptorLayout == nullptr) {
            descriptorLayout = Application::getVireo().createDescriptorLayout(L"Scene");
            descriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
            descriptorLayout->build();
        }

        sceneUniformBuffer = Application::getVireo().createBuffer(
            vireo::BufferType::UNIFORM,
            sizeof(SceneUniform), 1,
            L"Scene Uniform");
        sceneUniformBuffer->map();

        descriptorSet = Application::getVireo().createDescriptorSet(descriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
    }

    void SceneData::update() {
        if (currentCamera && currentCamera->isUpdated()) {
            const auto sceneUniform = SceneUniform {
                .cameraPosition = currentCamera->getPositionGlobal(),
                .projection = currentCamera->getProjection(),
                .view = inverse(currentCamera->getTransformGlobal()),
                .viewInverse = currentCamera->getTransformGlobal(),
            };
            sceneUniformBuffer->write(&sceneUniform);
            currentCamera->updated--;
        }

        +

        // Update in GPU memory only the materials modified since the last frame
        // for (const auto& materialIndex : std::views::values(materialsIndices)) {
        //     const auto material = materials[materialIndex];
        //     if (material->isUpdated()) {
        //         auto materialUniform = MaterialUniform {};
        //         if (const auto *standardMaterial = dynamic_cast<const StandardMaterial *>(material.get())) {
        //             materialUniform.albedoColor = standardMaterial->getAlbedoColor();
        //         }
        //         materialsStorageBuffer->write(
        //             &materialUniform,
        //             sizeof(MaterialUniform),
        //             sizeof(MaterialUniform) * materialIndex);
        //         material->updated--;
        //     }
        // }
    }

    void SceneData::draw(
        const std::shared_ptr<vireo::CommandList>& commandList,
        const std::shared_ptr<vireo::Pipeline>& pipeline,
        const Samplers& samplers,
        const std::vector<vireo::DrawIndexedIndirectCommand>& commands,
        const std::shared_ptr<vireo::Buffer>& commandBuffer) const {
        auto& resourceManager = Application::getResourcesManager();
        const auto sets = std::vector<std::shared_ptr<const vireo::DescriptorSet>> {
            Application::getResourcesManager().getDescriptorSet(),
            getDescriptorSet(),
            samplers.getDescriptorSet()};
        commandList->setDescriptors(sets);
        commandList->bindPipeline(pipeline);
        commandList->bindDescriptors(pipeline, sets);
        commandList->bindIndexBuffer(XXX);
        for (const auto& command : commands) {
            commandList->drawIndexedIndirect(
                commandBuffer,
                0,
                commands.size(),
                sizeof(vireo::DrawIndexedIndirectCommand));
        }
    }

}