/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.frustum;

import std;
import lysa.math;
import lysa.nodes.node;
import lysa.nodes.mesh_instance;

export namespace lysa {

    /**
     * %A camera or light frustum
     */
    struct Frustum {
        /**
         * One plane of a frustum cube
         */
        struct Plane {
            float4 data; // normal + distance
            void normalize();
        };

        /**
         * Creates a frustum
         * @param planes output;
         * @param matrix Projection View Matrix
         */
        static void extractPlanes(Plane planes[6], const float4x4& matrix);

    };

}