/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.application_config;

import std;
import lysa.enums;

export namespace lysa {
    struct LayerCollideWith {
        uint32_t layer;
        std::vector<uint32_t> collideWith;
    };

    struct LayerCollisionTable {
        uint32_t layersCount;
        std::vector<LayerCollideWith> layersCollideWith;
    };

    /**
     * Global application configuration
     */
    struct ApplicationConfig {
        //! Directory to search for resources and compiled shaders
        std::filesystem::path appDir{"."};
        //! Layers vs Layers collision table
        LayerCollisionTable layerCollisionTable{};
        //! Default font name, the file must exist in the path
        std::string defaultFontName{"DefaultFont.ttf"};
        //! Default font size. See the Font class for the details.
        uint32_t defaultFontSize{20};
        //! Where to log messages using log()
        int loggingMode{LOGGING_MODE_NONE};
        //! Monitor index for the logging Window
        LogLevel logLevelMin = LogLevel::INFO;
    };
}
