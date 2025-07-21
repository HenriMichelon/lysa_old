/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.texture;

import lysa.application;

namespace lysa {

    Texture::Texture(const std::string &name):
        Resource{name} {
    }

    ImageTexture::ImageTexture(const std::shared_ptr<Image> &image, const uint32 samplerIndex) :
        Texture{image->getName()},
        image{image},
        samplerIndex{samplerIndex} {
    }

}
