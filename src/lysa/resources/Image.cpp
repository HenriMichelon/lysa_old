/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.image;

namespace lysa {
    Image::Image(
        const uint32_t width,
        const uint32_t height,
        const std::wstring & name):
        Resource{name},
        width{width},
        height{height} {
    };
}
