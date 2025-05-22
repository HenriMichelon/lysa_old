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
    vertexArray{
        vireo,
        sizeof(VertexData),
        config.maxVertexInstances,
        config.maxStagingVertexInstances,
        vireo::BufferType::DEVICE_STORAGE,
        L"Vertex Array"},
    materialArray{
        vireo,
        sizeof(MaterialData),
        config.maxMaterialInstances,
        config.maxStagingMaterialInstances,
        vireo::BufferType::DEVICE_STORAGE,
        L"Material Array"} {
    }

    void Resources::cleanup() {
        vertexArray.cleanup();
        materialArray.cleanup();
    }

    void Resources::flush(const vireo::CommandList& commandList) {
        vertexArray.flush(commandList);
        materialArray.flush(commandList);
    }

}