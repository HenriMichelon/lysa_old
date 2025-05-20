/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.resources_manager;

import lysa.application;
import lysa.resources.material;

namespace lysa {
    Resources::Resources(const vireo::Vireo& vireo, ResourcesConfiguration& config):
    config{config},
    transferQueue{vireo.createSubmitQueue(vireo::CommandType::TRANSFER, L"Resources transfer")},
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
        sizeof(uint32),
        config.maxIndexInstances,
        config.maxStagingIndexInstances,
        vireo::BufferType::STORAGE,
        L"Index Array"},
    materialArray{
        vireo,
        sizeof(MaterialData),
        config.maxMaterialInstances,
        config.maxStagingMaterialInstances,
        vireo::BufferType::STORAGE,
        L"Material Array"},
    meshSurfaceArray{
        vireo,
        sizeof(MeshSurfaceData),
        config.maxMeshSurfacesInstances,
        config.maxStagingMeshSurfacesInstances,
        vireo::BufferType::STORAGE,
        L"MeshSurface Array"} {
        if (descriptorLayout == nullptr) {
            descriptorLayout = vireo.createDescriptorLayout(L"Resources");
            descriptorLayout->add(BINDING_VERTEX, vireo::DescriptorType::STORAGE);
            descriptorLayout->add(BINDING_INDEX, vireo::DescriptorType::STORAGE);
            descriptorLayout->add(BINDING_MATERIAL, vireo::DescriptorType::STORAGE);
            descriptorLayout->add(BINDING_MESH_SURFACE, vireo::DescriptorType::STORAGE);
            descriptorLayout->build();
        }
        descriptorSet = vireo.createDescriptorSet(descriptorLayout, L"Resources");
        descriptorSet->update(BINDING_VERTEX, vertexArray.getBuffer());
        descriptorSet->update(BINDING_INDEX, indexArray.getBuffer());
        descriptorSet->update(BINDING_MATERIAL, materialArray.getBuffer());
        descriptorSet->update(BINDING_MESH_SURFACE, meshSurfaceArray.getBuffer());
    }

    void Resources::cleanup() {
        waitIdle();
        vertexArray.cleanup();
        indexArray.cleanup();
        materialArray.cleanup();
        meshSurfaceArray.cleanup();
        commandAllocator.reset();
        commandList.reset();
        descriptorSet.reset();
        transferQueue.reset();
    }

    void Resources::flush() {
        commandList->begin();
        vertexArray.flush(commandList);
        indexArray.flush(commandList);
        materialArray.flush(commandList);
        meshSurfaceArray.flush(commandList);
        commandList->end();
        transferQueue->submit({commandList});
        transferQueue->waitIdle();
    }

    void Resources::waitIdle() const {
        transferQueue->waitIdle();
    }

}