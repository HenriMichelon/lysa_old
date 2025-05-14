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

    /**
     * Nodes state when the scene is paused or running
     */
    enum class ProcessMode {
        //! Inherits mode from the node's parent. This is the default for any newly created node
        INHERIT = 0,
        //! Stops processing when Application::isPaused() is true. This is the inverse of PROCESS_MODE_WHEN_PAUSED
        PAUSABLE = 1,
        //! Process only when Application::isPaused() is true. This is the inverse of PROCESS_MODE_PAUSABLE
        WHEN_PAUSED = 2,
        //! Always process. Keeps processing, ignoring Application::isPaused(). This is the inverse of PROCESS_MODE_DISABLED
        ALWAYS = 3,
        //! Never process. Completely disables processing, ignoring Application::isPaused(). This is the inverse of PROCESS_MODE_ALWAYS
        DISABLED = 4,
    };
}
