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
        const bool isForScene,
        const DeviceMemoryArray& meshInstancesArray) {
        const auto& vireo = Application::getVireo();
        globalBuffer = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(Global), 1, DEBUG_NAME);
        globalBuffer->map();
        commandClearCounterBuffer = vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, sizeof(uint32));
        constexpr auto clearValue = 0;
        commandClearCounterBuffer->map();
        commandClearCounterBuffer->write(&clearValue);
        commandClearCounterBuffer->unmap();

        downloadCounterBuffer = vireo.createBuffer(vireo::BufferType::BUFFER_DOWNLOAD, sizeof(uint32));
        downloadCounterBuffer->map();

        descriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        descriptorLayout->add(BINDING_GLOBAL, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_MESHINSTANCES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_INSTANCES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_INPUT, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_OUTPUT, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->add(BINDING_COUNTER, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->build();

        descriptorSet = vireo.createDescriptorSet(descriptorLayout, DEBUG_NAME);
        descriptorSet->update(BINDING_GLOBAL, globalBuffer);
        descriptorSet->update(BINDING_MESHINSTANCES, meshInstancesArray.getBuffer());

        const auto pipelineResources = vireo.createPipelineResources(
            { descriptorLayout },
            {},
            DEBUG_NAME);
        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();
        VirtualFS::loadBinaryData(L"app://" + Application::getConfiguration().shaderDir + L"/" +
            (isForScene ? SHADER_SCENE : SHADER_SHADOWMAP)
            + ext, tempBuffer);
        const auto shader = vireo.createShaderModule(tempBuffer);
        pipeline = vireo.createComputePipeline(pipelineResources, shader, DEBUG_NAME);
    }

    void FrustumCulling::dispatch(
        vireo::CommandList& commandList,
        const uint32 drawCommandsCount,
        const float4x4& view,
        const float4x4& projection,
        const vireo::Buffer& instances,
        const vireo::Buffer& input,
        const vireo::Buffer& output,
        const vireo::Buffer& counter) {
        if (drawCommandsCount == 0) { return; }
        auto global = Global{
            .drawCommandsCount = drawCommandsCount,
            .viewMatrix = inverse(view),
        };
        Frustum::extractPlanes(global.planes, mul(global.viewMatrix, projection));
        globalBuffer->write(&global);
        commandList.barrier(
            counter,
            vireo::ResourceState::INDIRECT_DRAW,
            vireo::ResourceState::COPY_DST);
        commandList.copy(*commandClearCounterBuffer, counter);
        commandList.barrier(
            counter,
            vireo::ResourceState::COPY_DST,
            vireo::ResourceState::COMPUTE_WRITE);

        descriptorSet->update(BINDING_INSTANCES, instances);
        descriptorSet->update(BINDING_INPUT, input);
        descriptorSet->update(BINDING_OUTPUT, output, counter);
        descriptorSet->update(BINDING_COUNTER, counter);

        commandList.barrier(
            input,
            vireo::ResourceState::INDIRECT_DRAW,
            vireo::ResourceState::COMPUTE_READ);
        commandList.barrier(
            output,
            vireo::ResourceState::INDIRECT_DRAW,
            vireo::ResourceState::COMPUTE_WRITE);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors({ descriptorSet });
        commandList.dispatch((drawCommandsCount + 63) / 64, 1, 1);
        commandList.barrier(
            output,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::INDIRECT_DRAW);
        commandList.barrier(
            input,
            vireo::ResourceState::COMPUTE_READ,
            vireo::ResourceState::INDIRECT_DRAW);

        commandList.barrier(
            counter,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::COPY_SRC);
        commandList.copy(counter, *downloadCounterBuffer);
        commandList.barrier(
            counter,
            vireo::ResourceState::COPY_SRC,
            vireo::ResourceState::INDIRECT_DRAW);
    }

    uint32 FrustumCulling::getDrawCommandsCount() const {
        return *reinterpret_cast<uint32*>(downloadCounterBuffer->getMappedAddress());
    }

}