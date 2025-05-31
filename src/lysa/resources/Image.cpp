/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.image;

import lysa.application;

namespace lysa {

    Image::Image(const std::shared_ptr<vireo::Image>& image, const std::wstring & name):
        Resource{name},
        image{image},
        index{Application::getResources().addTexture(*this)} {
    }

}
