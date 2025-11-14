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

    /**
     * Describes which layers can collide with a given layer.
     * Used to build a collision filtering table at engine initialization.
     */
    struct LayerCollideWith {
        /** The source collision layer. */
        collision_layer layer;
        /** List of layers that are allowed to collide with 'layer'. */
        std::vector<uint32> collideWith;
    };

    /**
     * Flattened description of layer‑to‑layer collision rules.
     */
    struct LayerCollisionTable {
        /** Total number of layers defined in the project. */
        uint32 layersCount;
        /** Per‑layer entries describing which other layers they collide with. */
        std::vector<LayerCollideWith> layersCollideWith;
    };

    /**
     * High‑level physics engine configuration.
     */
    struct PhysicsConfiguration {
        /** Layers vs Layers collision table. */
        LayerCollisionTable layerCollisionTable;
    };
}
