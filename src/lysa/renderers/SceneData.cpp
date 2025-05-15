/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.renderers.scene_data;

namespace lysa {

    SceneData::SceneData(const std::shared_ptr<vireo::Vireo>& vireo, const vireo::Extent &extent):
        Scene{extent},
        vireo{vireo} {
        if (descriptorLayout == nullptr) {
            descriptorLayout = vireo->createDescriptorLayout();
            descriptorLayout->add(BINDING_SCENE, vireo::DescriptorType::UNIFORM);
            descriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::UNIFORM);
            descriptorLayout->build();
        }

        sceneUniformBuffer = vireo->createBuffer(vireo::BufferType::UNIFORM, sizeof(SceneUniform), 1, L"Scene Uniform");
        sceneUniformBuffer->map();

        descriptorSet = vireo->createDescriptorSet(descriptorLayout, L"Scene");
        descriptorSet->update(BINDING_SCENE, sceneUniformBuffer);
    }

    void SceneData::update() {
        if (currentCamera) {
            sceneUniform.cameraPosition = currentCamera->getPositionGlobal();
            sceneUniform.projection = currentCamera->getProjection();
            sceneUniform.view = currentCamera->getView();
            sceneUniform.viewInverse = inverse(currentCamera->getView());
            sceneUniformBuffer->write(&sceneUniform);
        }

        if (modelsUpdated && modelUniformSize < models.size()) {
            modelUniformSize = models.size();
            modelUniforms = std::make_unique<ModelUniform[]>(models.size());
            modelUniformBuffers = vireo->createBuffer(vireo::BufferType::UNIFORM, sizeof(ModelUniform) * modelUniformSize, 1, L"Models Uniform");
            modelUniformBuffers->map();
            descriptorSet->update(BINDING_MODELS, modelUniformBuffers);
        }
        modelsUpdated = false;

        uint32_t modelIndex = 0;
        for (const auto &meshInstance : models) {
            modelUniforms[modelIndex].transform = meshInstance->getTransformGlobal();
            modelIndex++;
        }
        modelUniformBuffers->write(modelUniforms.get());
    }

}