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

        cullingDescriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        cullingDescriptorLayout->add(BINDING_CULLING_GLOBAL, vireo::DescriptorType::UNIFORM);
        cullingDescriptorLayout->add(BINDING_CULLING_MODELS, vireo::DescriptorType::DEVICE_STORAGE);
        cullingDescriptorLayout->add(BINDING_CULLING_MATERIALS, vireo::DescriptorType::DEVICE_STORAGE);
        cullingDescriptorLayout->add(BINDING_CULLING_SURFACE, vireo::DescriptorType::DEVICE_STORAGE);
        cullingDescriptorLayout->add(BINDING_CULLING_OUTPUT, vireo::DescriptorType::READWRITE_STORAGE);
        cullingDescriptorLayout->add(BINDING_CULLING_COUNTER, vireo::DescriptorType::READWRITE_STORAGE);
        cullingDescriptorLayout->build();

        cullingDescriptorSet = vireo.createDescriptorSet(cullingDescriptorLayout, DEBUG_NAME);
        cullingDescriptorSet->update(BINDING_CULLING_GLOBAL, globalBuffer);
        cullingDescriptorSet->update(BINDING_CULLING_MODELS, modelsArray.getBuffer());
        cullingDescriptorSet->update(BINDING_CULLING_MATERIALS, Application::getResources().getMaterialArray().getBuffer());
        cullingDescriptorSet->update(BINDING_CULLING_SURFACE, surfacesArray.getBuffer());

        commandDescriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        commandDescriptorLayout->add(BINDING_COMMAND_INDICES, vireo::DescriptorType::DEVICE_STORAGE);
        commandDescriptorLayout->add(BINDING_COMMAND_SURFACES, vireo::DescriptorType::DEVICE_STORAGE);
        commandDescriptorLayout->add(BINDING_COMMAND_INPUT, vireo::DescriptorType::READWRITE_STORAGE);
        commandDescriptorLayout->add(BINDING_COMMAND_COUNTER, vireo::DescriptorType::READWRITE_STORAGE);
        commandDescriptorLayout->add(BINDING_COMMAND_OUTPUT, vireo::DescriptorType::READWRITE_STORAGE);
        commandDescriptorLayout->add(BINDING_COMMAND_COMMAND, vireo::DescriptorType::READWRITE_STORAGE);
        commandDescriptorLayout->build();

        commandDescriptorSet = vireo.createDescriptorSet(commandDescriptorLayout, DEBUG_NAME);
        commandDescriptorSet->update(BINDING_COMMAND_INDICES, Application::getResources().getIndexArray().getBuffer());
        commandDescriptorSet->update(BINDING_COMMAND_SURFACES, surfacesArray.getBuffer());

        const auto pipelineResourcesCulling = vireo.createPipelineResources(
            { cullingDescriptorLayout },
            {},
            DEBUG_NAME + L" culling");
        const auto pipelineResourcesCommand = vireo.createPipelineResources(
            { commandDescriptorLayout },
            {},
            DEBUG_NAME + L" draw commands");

        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();

        VirtualFS::loadBinaryData(L"app://shaders/frustum_culling.comp" + ext, tempBuffer);
        auto shader = vireo.createShaderModule(tempBuffer);
        pipelineCulling = vireo.createComputePipeline(pipelineResourcesCulling, shader, DEBUG_NAME + L" culling");

        VirtualFS::loadBinaryData(L"app://shaders/draw_commands.comp" + ext, tempBuffer);
        shader = vireo.createShaderModule(tempBuffer);
        pipelineDrawCommand = vireo.createComputePipeline(pipelineResourcesCommand, shader, DEBUG_NAME + L" draw commands");
    }

    void FrustumCulling::dispatch(
        vireo::CommandList& commandList,
        const pipeline_id pipelineId,
        const uint32 surfaceCount,
        const Camera& camera,
        const vireo::Buffer& outputSurfaces,
        const vireo::Buffer& outputSurfacesCounter,
        const vireo::Buffer& outputIndices,
        const vireo::Buffer& command) {

        auto global = Global{
            .pipelineId = pipelineId,
            .surfaceCount = surfaceCount,
        };
        Frustum::extractPlanes(global.planes, mul(inverse(camera.getTransformGlobal()), camera.getProjection()));
        globalBuffer->write(&global);

        commandList.copy(*commandClearBuffer, outputSurfacesCounter);

        commandList.barrier(
            outputSurfaces,
            vireo::ResourceState::SHADER_READ,
            vireo::ResourceState::COMPUTE_WRITE);
        cullingDescriptorSet->update(BINDING_CULLING_OUTPUT, outputSurfaces);
        cullingDescriptorSet->update(BINDING_CULLING_COUNTER, outputSurfacesCounter);
        commandList.bindPipeline(pipelineCulling);
        commandList.bindDescriptors(pipelineCulling, { cullingDescriptorSet });
        commandList.dispatch((surfaceCount + 63) / 64, 1, 1);
        commandList.barrier(
            outputSurfaces,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::SHADER_READ);

        commandList.barrier(
            command,
            vireo::ResourceState::INDIRECT_DRAW,
            vireo::ResourceState::COPY_DST);
        commandList.copy(*commandClearBuffer, command);
        commandList.barrier(
            command,
            vireo::ResourceState::COPY_DST,
            vireo::ResourceState::COMPUTE_WRITE);

        commandList.barrier(
            outputIndices,
            vireo::ResourceState::SHADER_READ,
            vireo::ResourceState::COMPUTE_WRITE);
        commandDescriptorSet->update(BINDING_COMMAND_INPUT, outputSurfaces);
        commandDescriptorSet->update(BINDING_COMMAND_COUNTER, outputSurfacesCounter);
        commandDescriptorSet->update(BINDING_COMMAND_OUTPUT, outputIndices);
        commandDescriptorSet->update(BINDING_COMMAND_COMMAND, command);
        commandList.bindPipeline(pipelineDrawCommand);
        commandList.bindDescriptors(pipelineDrawCommand, { commandDescriptorSet });
        commandList.dispatch(1, 1, 1);
        commandList.barrier(
            outputIndices,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::SHADER_READ);

        commandList.barrier(
            command,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::INDIRECT_DRAW);
    }

}