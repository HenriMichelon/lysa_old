/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.resources_manager;

import lysa.application;

namespace lysa {
    ResourcesManager::ResourcesManager(const vireo::Vireo& vireo, ResourcesConfiguration& config):
    config{config},
    transferQueue{vireo.createSubmitQueue(vireo::CommandType::TRANSFER, L"Resources")},
    commandAllocator{vireo.createCommandAllocator(vireo::CommandType::TRANSFER)},
    commandList{commandAllocator->createCommandList()},
    vertexArray{
        vireo,
        sizeof(Vertex),
        config.maxVertexInstances,
        config.maxStagingVertexInstances,
        vireo::BufferType::STORAGE,
        L"Vertex Array"},
    indexArray{
        vireo,
        sizeof(Vertex),
        config.maxIndexInstances,
        config.maxStagingIndexInstances,
        vireo::BufferType::STORAGE,
        L"Index Array"} {
        if (descriptorLayout == nullptr) {
            descriptorLayout->add(BINDING_VERTEX, vireo::DescriptorType::STORAGE);
            descriptorLayout->add(BINDING_INDEX, vireo::DescriptorType::STORAGE);
            descriptorLayout->build();
        }
        descriptorSet = vireo.createDescriptorSet(descriptorLayout, L"Resources");
        descriptorSet->update(BINDING_VERTEX, vertexArray.getBuffer());
        descriptorSet->update(BINDING_INDEX, indexArray.getBuffer());
    }

    ResourcesManager::~ResourcesManager() {
        waitIdle();
    }

    void ResourcesManager::flush() {
        commandList->begin();
        vertexArray.flush(commandList);
        indexArray.flush(commandList);
        commandList->end();
        transferQueue->submit({commandList});
        transferQueue->waitIdle();
    }

    void ResourcesManager::waitIdle() const {
        transferQueue->waitIdle();
    }


}