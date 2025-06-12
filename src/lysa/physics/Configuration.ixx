/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.physics.configuration;

import std;
import lysa.types;

export namespace lysa {

    struct LayerCollideWith {
        collision_layer layer;
        std::vector<uint32> collideWith;
    };

    struct LayerCollisionTable {
        uint32 layersCount;
        std::vector<LayerCollideWith> layersCollideWith;
    };

    struct PhysicsConfiguration {
        //! Layers vs Layers collision table
        LayerCollisionTable layerCollisionTable;
    };
}
