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
        const DeviceMemoryArray& modelsArray,
        const DeviceMemoryArray& surfacesArray) {
        const auto& vireo = Application::getVireo();
        globalBuffer = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(Global), 1, DEBUG_NAME);
        globalBuffer->map();
        commandClearBuffer = vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, sizeof(vireo::DrawIndirectCommand));
        constexpr auto clearValue = vireo::DrawIndirectCommand{};
        commandClearBuffer->map();
        commandClearBuffer->write(&clearValue);
        commandClearBuffer->unmap();

        descriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        descriptorLayout->add(BINDING_GLOBAL, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_INDICES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_MATERIALS, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_SURFACES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_OUTPUT, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->add(BINDING_COMMAND, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->build();

        descriptorSet = vireo.createDescriptorSet(descriptorLayout, DEBUG_NAME);
        descriptorSet->update(BINDING_GLOBAL, globalBuffer);
        descriptorSet->update(BINDING_MODELS, modelsArray.getBuffer());
        descriptorSet->update(BINDING_INDICES, Application::getResources().getIndexArray().getBuffer());
        descriptorSet->update(BINDING_MATERIALS, Application::getResources().getMaterialArray().getBuffer());
        descriptorSet->update(BINDING_SURFACES, surfacesArray.getBuffer());

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
        auto global = Global{
            .pipelineId = pipelineId,
            .surfaceCount = surfaceCount,
        };
        Frustum::extractPlanes(global.planes, mul(inverse(camera.getTransformGlobal()), camera.getProjection()));
        globalBuffer->write(&global);
        commandList.barrier(
            command,
            vireo::ResourceState::INDIRECT_DRAW,
            vireo::ResourceState::COPY_DST);
        commandList.copy(*commandClearBuffer, command);
        commandList.barrier(
            command,
            vireo::ResourceState::COPY_DST,
            vireo::ResourceState::COMPUTE_WRITE);
        descriptorSet->update(BINDING_OUTPUT, output);
        descriptorSet->update(BINDING_COMMAND, command);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors(pipeline, { descriptorSet });
        commandList.dispatch((surfaceCount + 63) / 64, 1, 1);
        commandList.barrier(
            command,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::INDIRECT_DRAW);
    }

}