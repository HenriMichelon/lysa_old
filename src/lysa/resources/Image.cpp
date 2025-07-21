/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cstring>
#include <stb_image_write.h>
module lysa.resources.image;

import lysa.application;
import lysa.log;
import lysa.virtual_fs;

namespace lysa {

    Image::Image(const std::shared_ptr<vireo::Image>& image, const std::string & name):
        Resource{name},
        image{image},
        index{Application::getResources().addTexture(*this)} {
    }

    std::shared_ptr<Image> Image::create(
        const void* data,
        const uint32 width, const uint32 height,
        const vireo::ImageFormat imageFormat,
        const std::string& name) {
        const auto& vireo = Application::getVireo();
        const auto image = vireo.createImage(imageFormat, width, height, 1, 1, name);

        const auto commandAllocator = vireo.createCommandAllocator(vireo::CommandType::GRAPHIC);
        const auto commandList = commandAllocator->createCommandList();
        commandList->begin();
        commandList->barrier(image, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
        commandList->upload(image, data);
        commandList->barrier(image, vireo::ResourceState::COPY_DST, vireo::ResourceState::SHADER_READ);
        commandList->end();

        const auto& graphicQueue = Application::getGraphicQueue();
        graphicQueue->submit({commandList});
        graphicQueue->waitIdle();

        return std::make_shared<Image>(image, name);
    }

    std::shared_ptr<Image> Image::load(
        const std::string &filepath,
        const vireo::ImageFormat imageFormat) {
        uint32 texWidth, texHeight;
        uint64 imageSize;
        auto *pixels = VirtualFS::loadRGBAImage(filepath, texWidth, texHeight, imageSize);
        if (!pixels) { throw Exception("failed to load texture image ", filepath); }
        auto image = create(pixels, texWidth, texHeight, imageFormat, filepath);
        VirtualFS::destroyImage(pixels);
        return image;
    }

    void Image::save(const std::string& filepath) const {
        save(filepath, image);
    }

    void Image::save(const std::string& filepath, const std::shared_ptr<vireo::Image>& image) {
        const auto& vireo = Application::getVireo();
        const auto&graphicQueue = Application::getGraphicQueue();
        graphicQueue->waitIdle();

        const auto commandAllocator = vireo.createCommandAllocator(vireo::CommandType::GRAPHIC);
        const auto commandList = commandAllocator->createCommandList();
        commandList->begin();
        const auto buffer = vireo.createBuffer(vireo::BufferType::IMAGE_DOWNLOAD, image->getAlignedImageSize());
        commandList->copy(image, buffer);
        commandList->end();
        graphicQueue->submit({commandList});
        graphicQueue->waitIdle();

        buffer->map();
        const auto rowPitch = image->getRowPitch();
        const auto alignedRowPitch = image->getAlignedRowPitch();
        std::vector<uint8> imageData(image->getImageSize());
        const auto* source = static_cast<uint8*>(buffer->getMappedAddress());
        for (int y = 0; y < image->getHeight(); ++y) {
            memcpy(&imageData[y * rowPitch], &source[y * alignedRowPitch], rowPitch);
        }
        buffer->unmap();

        if (filepath.ends_with(".hdr")) {
            const auto floatImage = reinterpret_cast<const float*>(imageData.data());
            stbi_write_hdr(filepath.c_str(),
                image->getWidth(),
                image->getHeight(),
                1,
                floatImage);
        } else if (filepath.ends_with(".png")) {
            stbi_write_png(filepath.c_str(),
                image->getWidth(),
                image->getHeight(),
                image->getPixelSize(image->getFormat()),
                imageData.data(),
                rowPitch);
        }
    }

}
