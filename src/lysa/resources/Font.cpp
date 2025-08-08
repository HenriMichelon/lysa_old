/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <stb_image_write.h>
module lysa.resources.font;

import vireo;
import lysa.application;
import lysa.constants;
import lysa.exception;
import lysa.global;
import lysa.log;
import lysa.virtual_fs;
import lysa.window;

namespace lysa {

    void Font::getSize(const std::string &text, float &width, float &height) {

    }

    Font::Font(const Font &font, const uint32 size, Window* window) : Font{font.getFontName(), size, window} {
    }

}
