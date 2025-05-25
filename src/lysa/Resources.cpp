/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.resources;

import lysa.application;
import lysa.resources.material;

namespace lysa {
    Resources::Resources(const vireo::Vireo& vireo, ResourcesConfiguration& config):
    config{config},
    vertexArray {
        vireo,
        sizeof(VertexData),
        config.maxVertexInstances,
        config.maxStagingVertexInstances,
        vireo::BufferType::DEVICE_STORAGE,
        L"Vertex Array"},
    materialArray {
        vireo,
        sizeof(MaterialData),
        config.maxMaterialInstances,
        config.maxStagingMaterialInstances,
        vireo::BufferType::DEVICE_STORAGE,
        L"Material Array"} {
        if (descriptorLayout == nullptr) {
            descriptorLayout = vireo.createDescriptorLayout(L"Resources");
            descriptorLayout->add(BINDING_VERTEX, vireo::DescriptorType::STORAGE);
            descriptorLayout->add(BINDING_MATERIAL, vireo::DescriptorType::STORAGE);
            descriptorLayout->build();
        }
        descriptorSet = vireo.createDescriptorSet(descriptorLayout, L"Resources");
        descriptorSet->update(BINDING_VERTEX, vertexArray.getBuffer());
        descriptorSet->update(BINDING_MATERIAL, materialArray.getBuffer());
    }

    void Resources::cleanup() {
        vertexArray.cleanup();
        materialArray.cleanup();
        descriptorLayout.reset();
        descriptorSet.reset();
    }

    void Resources::flush(const vireo::CommandList& commandList) {
        auto lock = std::unique_lock(mutex, std::try_to_lock);
        vertexArray.flush(commandList);
        materialArray.flush(commandList);
        updated = false;
    }

}