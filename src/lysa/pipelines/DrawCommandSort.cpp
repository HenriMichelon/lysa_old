/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.pipelines.draw_command_sort;

import lysa.application;
import lysa.virtual_fs;

namespace lysa {
    DrawCommandSort::DrawCommandSort() {
        const auto& vireo = Application::getVireo();
        globalBuffer = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(Global), 1, DEBUG_NAME);
        globalBuffer->map();

        descriptorLayout = vireo.createDescriptorLayout(DEBUG_NAME);
        descriptorLayout->add(BINDING_GLOBAL, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_INPUT, vireo::DescriptorType::DEVICE_STORAGE);
        descriptorLayout->build();

        descriptorSet = vireo.createDescriptorSet(descriptorLayout, DEBUG_NAME);
        descriptorSet->update(BINDING_GLOBAL, globalBuffer);

        const auto pipelineResources = vireo.createPipelineResources(
            { descriptorLayout },
            {},
            DEBUG_NAME);
        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();
        VirtualFS::loadBinaryData(L"app://" + Application::getConfiguration().shaderDir + L"/mesh_sort.comp" + ext, tempBuffer);
        const auto shader = vireo.createShaderModule(tempBuffer);
        pipeline = vireo.createComputePipeline(pipelineResources, shader, DEBUG_NAME);
    }

    void DrawCommandSort::dispatch(
        vireo::CommandList& commandList,
        const uint32 drawCommandsCount,
        const vireo::Buffer& input) {
        if (drawCommandsCount == 0) { return; }
        auto global = Global{
            .drawCommandsCount = drawCommandsCount,
        };
        globalBuffer->write(&global);
        descriptorSet->update(BINDING_INPUT, input);
        commandList.barrier(
            input,
            vireo::ResourceState::INDIRECT_DRAW,
            vireo::ResourceState::COMPUTE_WRITE);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors({ descriptorSet });
        commandList.dispatch((drawCommandsCount + 63) / 64, 1, 1);
        commandList.barrier(
            input,
            vireo::ResourceState::COMPUTE_WRITE,
            vireo::ResourceState::INDIRECT_DRAW);
    }

}