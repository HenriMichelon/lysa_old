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
     * Rendering Window mode
     */
    enum class WindowMode : uint8_t {
        //! A Window with a border and a title that can be minimized
        WINDOWED = 0,
        //! A maximized Window with a border and a title that can be minimized
        WINDOWED_MAXIMIZED = 1,
        //! A maximized Window without a border and without a title
        WINDOWED_FULLSCREEN = 2,
        //! A full-screen Window. The screen resolution will be changed
        FULLSCREEN = 3,
    };

    /**
     * Where to log messages
     */
    enum LoggingMode : uint32_t {
        //! Disable logging
        LOGGING_MODE_NONE = 0,
        //! Log the messages into a file named 'lysa_log.txt'
        LOGGING_MODE_FILE = 0x010,
        //! Log the messages to the standard output.
        LOGGING_MODE_STDOUT = 0x100
    };

    /**
     * Log levels
     */
#undef ERROR
    enum class LogLevel : int32_t {
        TRACE = -1,
        DEBUG = 0,
        INFO = 1,
        GAME1 = 2,
        GAME2 = 3,
        GAME3 = 4,
        WARNING = 5,
        ERROR = 6,
        CRITICAL = 7,
        INTERNAL = 100,
    };

}
