/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.resources_manager;

import lysa.application;
import lysa.resources.mesh;

namespace lysa {
    ResourcesManager::ResourcesManager(const vireo::Vireo& vireo, ResourcesConfiguration& config):
    config{config},
    transferQueue{vireo.createSubmitQueue(vireo::CommandType::GRAPHIC, L"Resources Queue")},
    vertexArray{
        sizeof(Vertex),
        config.maxVertexInstances,
        config.maxStagingVertexInstances,
        vireo::BufferType::VERTEX,
        L"Vertex Array"},
    indexArray{
        sizeof(Vertex),
        config.maxIndexInstances,
        config.maxStagingIndexInstances,
        vireo::BufferType::INDEX,
        L"Index Array"} {
        }

    ResourcesManager::~ResourcesManager() {
        waitIdle();
    }

    void ResourcesManager::waitIdle() const {
        transferQueue->waitIdle();
    }

    void ResourcesManager::upload(MemoryArray& memoryArray) const {
        const auto allocator = Application::getVireo().createCommandAllocator(vireo::CommandType::TRANSFER);
        const auto commandList = allocator->createCommandList();
        commandList->begin();
        memoryArray.flush(commandList);
        commandList->end();
        transferQueue->submit({commandList});
        transferQueue->waitIdle();
    }

}