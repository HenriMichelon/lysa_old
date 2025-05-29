/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.texture;

namespace lysa {

    Texture::Texture(const std::wstring &name):
        Resource{name} {
    }

    ImageTexture::ImageTexture(const std::shared_ptr<Image> &image):
        Texture{image->getName()},
        image{image} {
    }

    // ImageTexture::ImageTexture(const std::wstring &filename, const vireo::ImageFormat imageFormat):
    //     Texture{filename},
    //     image{Image::load(filename, imageFormat)} {
    // }

}
