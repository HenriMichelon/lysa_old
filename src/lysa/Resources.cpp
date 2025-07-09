/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <stb_image_write.h>
module lysa.resources;

import lysa.application;
import lysa.log;
import lysa.resources.material;

namespace lysa {
    Resources::Resources(
        const vireo::Vireo& vireo,
        ResourcesConfiguration& config,
        const vireo::SubmitQueue& graphicQueue):
        config{config},
        vertexArray {
            vireo,
            sizeof(VertexData),
            config.maxVertexInstances,
            config.maxVertexInstances,
            vireo::BufferType::VERTEX,
            L"Vertex Array"},
        indexArray {
            vireo,
            sizeof(uint32),
            config.maxIndexInstances,
            config.maxIndexInstances,
            vireo::BufferType::INDEX,
            L"Index Array"},
        materialArray {
            vireo,
            sizeof(MaterialData),
            config.maxMaterialInstances,
            config.maxMaterialInstances,
            vireo::BufferType::DEVICE_STORAGE,
            L"Material Array"},
        meshSurfaceArray {
            vireo,
            sizeof(MeshSurfaceData),
            config.maxMeshSurfaceInstances,
            config.maxMeshSurfaceInstances,
            vireo::BufferType::DEVICE_STORAGE,
            L"MeshSurface Array"},
        samplers{vireo},
        textures(MAX_TEXTURES) {
        if (descriptorLayout == nullptr) {
            descriptorLayout = vireo.createDescriptorLayout(L"Resources");
            descriptorLayout->add(BINDING_MATERIAL, vireo::DescriptorType::DEVICE_STORAGE);
            descriptorLayout->add(BINDING_SURFACES, vireo::DescriptorType::DEVICE_STORAGE);
            descriptorLayout->add(BINDING_TEXTURE, vireo::DescriptorType::SAMPLED_IMAGE, textures.size());
            descriptorLayout->build();
        }

        auto blankJPEG = createBlankJPEG();
        std::vector<void*> cubeFaces(6);
        for (int i = 0; i < 6; i++) {
            cubeFaces[i]= blankJPEG.data();
        }

        blankImage = vireo.createImage(
            vireo::ImageFormat::R8G8B8A8_SRGB,
            1, 1,1, 1,
            L"Blank Image");
        blankCubeMap = vireo.createImage(
            vireo::ImageFormat::R8G8B8A8_SRGB,
            1, 1,1, 6,
            L"Blank CubeMap");
        const auto commandAllocator = vireo.createCommandAllocator(vireo::CommandType::GRAPHIC);
        const auto commandList = commandAllocator->createCommandList();
        commandList->begin();
        commandList->barrier(
            blankImage,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::COPY_DST);
        commandList->upload(blankImage, blankJPEG.data());
        commandList->barrier(
            blankImage,
            vireo::ResourceState::COPY_DST,
            vireo::ResourceState::SHADER_READ);
        commandList->barrier(
            blankCubeMap,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::COPY_DST);
        commandList->uploadArray(blankCubeMap, cubeFaces);
        commandList->barrier(
            blankCubeMap,
            vireo::ResourceState::COPY_DST,
            vireo::ResourceState::SHADER_READ);
        commandList->end();
        graphicQueue.submit({commandList});
        graphicQueue.waitIdle();

        for (int i = 0; i < textures.size(); i++) {
            textures[i] = blankImage;
        }

        descriptorSet = vireo.createDescriptorSet(descriptorLayout, L"Resources");
        descriptorSet->update(BINDING_MATERIAL, materialArray.getBuffer());
        descriptorSet->update(BINDING_SURFACES, meshSurfaceArray.getBuffer());
        descriptorSet->update(BINDING_TEXTURE, textures);
    }

    uint32 Resources::addTexture(const Image& image) {
        auto lock = std::lock_guard(mutex);
        for (uint32 index = 0; index < textures.size(); index++) {
            if (textures[index] == blankImage) {
                textures[index] = image.getImage();
                textureUpdated = true;
                return index;
            }
        }
        throw Exception("Out of memory for textures");
    }

    void Resources::update() {
        auto lock = std::lock_guard(mutex);
        if (textureUpdated) {
            Application::getGraphicQueue()->waitIdle();
            descriptorSet->update(BINDING_TEXTURE, textures);
            textureUpdated = false;
        }
        if (samplers.ipUpdated()) {
            samplers.update();
        }
    }

    void Resources::cleanup() {
        samplers.cleanup();
        textures.clear();
        blankImage.reset();
        blankCubeMap.reset();
        indexArray.cleanup();
        vertexArray.cleanup();
        materialArray.cleanup();
        meshSurfaceArray.cleanup();
        descriptorLayout.reset();
        descriptorSet.reset();
    }

    void Resources::flush() {
        auto lock = std::unique_lock(mutex, std::try_to_lock);
        const auto preBarriersCommand = Application::getSubmitQueue().beginOneTimeGraphicCommand();
        indexArray.preBarrier(*preBarriersCommand.commandList);
        vertexArray.preBarrier(*preBarriersCommand.commandList);
        materialArray.preBarrier(*preBarriersCommand.commandList);
        meshSurfaceArray.preBarrier(*preBarriersCommand.commandList);
        Application::getSubmitQueue().endOneTimeCommand(preBarriersCommand);
        const auto command = Application::getSubmitQueue().beginOneTimeTransferCommand();
        indexArray.flush(*command.commandList);
        vertexArray.flush(*command.commandList);
        materialArray.flush(*command.commandList);
        meshSurfaceArray.flush(*command.commandList);
        updated = false;
        Application::getSubmitQueue().endOneTimeCommand(command);
        const auto barriersCommand = Application::getSubmitQueue().beginOneTimeGraphicCommand();
        indexArray.postBarrier(*barriersCommand.commandList);
        vertexArray.postBarrier(*barriersCommand.commandList);
        materialArray.postBarrier(*barriersCommand.commandList);
        meshSurfaceArray.postBarrier(*barriersCommand.commandList);
        Application::getSubmitQueue().endOneTimeCommand(barriersCommand);
    }

    void Resources::stb_write_func(void *context, void *data, const int size) {
        auto *buffer = static_cast<std::vector<uint8> *>(context);
        auto *ptr    = static_cast<uint8*>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    std::vector<uint8> Resources::createBlankJPEG() {
        std::vector<uint8> blankJPEG;
        const auto data = new uint8[1 * 1 * 3];
        data[0]   = static_cast<uint8>(0);
        data[1]   = static_cast<uint8>(0);
        data[2]   = static_cast<uint8>(0);
        stbi_write_jpg_to_func(stb_write_func, &blankJPEG, 1, 1, 3, data, 100);
        delete[] data;
        return blankJPEG;
    }

}