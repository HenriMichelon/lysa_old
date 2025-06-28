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

namespace lysa {

    Image::Image(const std::shared_ptr<vireo::Image>& image, const std::wstring & name):
        Resource{name},
        image{image},
        index{Application::getResources().addTexture(*this)} {
    }

    void Image::save(const std::wstring& filepath) const {
        save(filepath, image);
    }

    void Image::save(const std::wstring& filepath, const std::shared_ptr<vireo::Image>& image) {
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

        if (filepath.ends_with(L".hdr")) {
            const auto floatImage = reinterpret_cast<const float*>(imageData.data());
            stbi_write_hdr(std::to_string(filepath).c_str(),
                image->getWidth(),
                image->getHeight(),
                1,
                floatImage);
        } else if (filepath.ends_with(L".png")) {
            stbi_write_png(std::to_string(filepath).c_str(),
                image->getWidth(),
                image->getHeight(),
                image->getPixelSize(image->getFormat()),
                imageData.data(),
                rowPitch);
        }
    }

}
