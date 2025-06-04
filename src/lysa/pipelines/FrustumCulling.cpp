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

        counterBuffer = vireo.createBuffer(vireo::BufferType::STORAGE, sizeof(uint32), 1, DEBUG_NAME + L" counter");
        counterBuffer->map();

        descriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        descriptorLayout->add(BINDING_GLOBAL, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_MODELS, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_MATERIALS, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_SURFACES, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_INPUT, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->add(BINDING_OUTPUT, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->add(BINDING_COUNTER, vireo::DescriptorType::READWRITE_STORAGE);
        descriptorLayout->build();

        descriptorSet = vireo.createDescriptorSet(descriptorLayout, DEBUG_NAME);
        descriptorSet->update(BINDING_GLOBAL, globalBuffer);
        descriptorSet->update(BINDING_MODELS, modelsArray.getBuffer());
        descriptorSet->update(BINDING_MATERIALS, Application::getResources().getMaterialArray().getBuffer());
        descriptorSet->update(BINDING_SURFACES, surfacesArray.getBuffer());
        descriptorSet->update(BINDING_COUNTER, counterBuffer);

        const auto pipelineResources = vireo.createPipelineResources(
            { descriptorLayout },
            {},
            DEBUG_NAME);
        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();
        VirtualFS::loadBinaryData(L"app://shaders/frustum_culling.comp" + ext, tempBuffer);
        const auto shader = vireo.createShaderModule(tempBuffer);
        pipeline = vireo.createComputePipeline(pipelineResources, shader, DEBUG_NAME);

        // commandAllocator = vireo.createCommandAllocator(vireo::CommandType::COMPUTE);
        // commandList = commandAllocator->createCommandList();
    }

    void FrustumCulling::dispatch(
        vireo::CommandList& commandList,
        const float aspectRatio,
        const pipeline_id pipelineId,
        const uint32 indexCount,
        const Camera& camera,
        const vireo::Buffer& input,
        const vireo::Buffer& output) {
        const auto frustum = Frustum{
            aspectRatio,
            camera,
            camera.getFov(),
            camera.getNearDistance(),
            camera.getFarDistance()
        };
        const auto global = Global{
            .pipelineId = pipelineId,
            .indexCount = indexCount,
            .planes = {
                frustum.farFace,
                frustum.nearFace,
                frustum.leftFace,
                frustum.rightFace,
                frustum.topFace,
                frustum.bottomFace,
            },
        };
        globalBuffer->write(&global);
        descriptorSet->update(BINDING_INPUT, input);
        descriptorSet->update(BINDING_OUTPUT, output);
        // commandAllocator->reset();
        // commandList->begin();
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors(pipeline, { descriptorSet });
        commandList.dispatch((indexCount + 63) / 64, 1, 1);
        // commandList->end();
        // Application::getComputeQueue()->submit({commandList});
    }

    uint32 FrustumCulling::getCounter() const {
        return *(reinterpret_cast<uint32*>(counterBuffer->getMappedAddress()));
    }


}