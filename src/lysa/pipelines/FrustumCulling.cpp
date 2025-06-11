/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.pipelines.frustum_culling;

import lysa.application;
import lysa.virtual_fs;

namespace lysa {
    FrustumCulling::FrustumCulling(
        const DeviceMemoryArray& meshInstancesArray,
        const DeviceMemoryArray& meshSurfacesArray) {
        const auto& vireo = Application::getVireo();
        globalBuffer = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(Global), 1, DEBUG_NAME);
        globalBuffer->map();
        commandClearCounterBuffer = vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, sizeof(uint32));
        constexpr auto clearValue = 0;
        commandClearCounterBuffer->map();
        commandClearCounterBuffer->write(&clearValue);
        commandClearCounterBuffer->unmap();

        descriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        descriptorLayout->add(BINDING_GLOBAL, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_INDICES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_MESHINSTANCES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_MESHSURFACES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_OUTPUT, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->add(BINDING_COUNTER, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->build();

        descriptorSet = vireo.createDescriptorSet(descriptorLayout, DEBUG_NAME);
        descriptorSet->update(BINDING_GLOBAL, globalBuffer);
        descriptorSet->update(BINDING_MESHINSTANCES, meshInstancesArray.getBuffer());
        descriptorSet->update(BINDING_INDICES, Application::getResources().getIndexArray().getBuffer());
        descriptorSet->update(BINDING_MESHSURFACES, meshSurfacesArray.getBuffer());

        const auto pipelineResources = vireo.createPipelineResources(
            { descriptorLayout },
            {},
            DEBUG_NAME);
        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();
        VirtualFS::loadBinaryData(L"app://shaders/frustum_culling.comp" + ext, tempBuffer);
        const auto shader = vireo.createShaderModule(tempBuffer);
        pipeline = vireo.createComputePipeline(pipelineResources, shader, DEBUG_NAME);
    }

    void FrustumCulling::dispatch(
        vireo::CommandList& commandList,
        const pipeline_id pipelineId,
        const uint32 surfaceCount,
        const Camera& camera,
        const vireo::Buffer& output,
        const vireo::Buffer& command) {
        // auto global = Global{
        //     .pipelineId = pipelineId,
        //     .surfaceCount = surfaceCount,
        // };
        // Frustum::extractPlanes(global.planes, mul(inverse(camera.getTransformGlobal()), camera.getProjection()));
        // globalBuffer->write(&global);
        // commandList.barrier(
        //     command,
        //     vireo::ResourceState::INDIRECT_DRAW,
        //     vireo::ResourceState::COPY_DST);
        // commandList.copy(*commandClearBuffer, command);
        // commandList.barrier(
        //     command,
        //     vireo::ResourceState::COPY_DST,
        //     vireo::ResourceState::COMPUTE_WRITE);
        // descriptorSet->update(BINDING_OUTPUT, output);
        // descriptorSet->update(BINDING_COMMAND, command);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors({ descriptorSet });
        commandList.dispatch((surfaceCount + 63) / 64, 1, 1);
        commandList.barrier(
            command,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::INDIRECT_DRAW);
    }

}