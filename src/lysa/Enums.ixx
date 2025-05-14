/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.enums;

import std;

export namespace lysa {
    /**
    * A Material transparency mode
    * Any transparency mode other than Transparency::DISABLED has a greater performance impact compared to opaque rendering.
    */
    enum class Transparency {
        //! The material will not use transparency. This is the fastest to render.
        DISABLED = 0,
        //! The material will use the texture's alpha values for transparency.
        ALPHA = 1,
        //! The material will cut off all values below a threshold, the rest will remain opaque.
        SCISSOR = 2,
        //! The material will cut off all values below a threshold, the rest will use the texture's alpha values for transparency.
        SCISSOR_ALPHA = 3,
    };
}
