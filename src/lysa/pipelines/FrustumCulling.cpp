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

        descriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        descriptorLayout->add(BINDING_GLOBAL, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_INDICES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_MATERIALS, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_SURFACES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_OUTPUT, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->add(BINDING_COUNTER, vireo::DescriptorType::READWRITE_STORAGE);
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
        const vireo::Buffer& counterBuffer) {
        auto global = Global{
            .pipelineId = pipelineId,
            .surfaceCount = surfaceCount,
        };
        Frustum::extractPlanes(global.planes, mul(camera.getProjection(), inverse(camera.getTransformGlobal())));
        globalBuffer->write(&global);
        descriptorSet->update(BINDING_OUTPUT, output);
        descriptorSet->update(BINDING_COUNTER, counterBuffer);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors(pipeline, { descriptorSet });
        commandList.dispatch((surfaceCount + 63) / 64, 1, 1);
    }

}