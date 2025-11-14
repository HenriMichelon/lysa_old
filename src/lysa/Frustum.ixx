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
     * Represents a view frustum used for visibility testing (frustum culling).
     *
     * %A frustum is defined by six clipping planes extracted from a combined
     * projection-view matrix (sometimes called clip-from-world). This utility
     * provides a compact plane representation and helpers to extract planes
     * from a matrix for culling meshes, lights, or other bounding volumes.
     */
    struct Frustum {
        /**
         * Single clipping plane of the frustum.
         */
        struct Plane {
            /**
             * Packed plane equation stored as (nx, ny, nz, d) where
             * n = (nx, ny, nz) is the plane unit normal and d is the signed
             * distance from the origin to the plane along the normal.
             * Points x satisfy: dot(n, x) + d = 0.
             */
            float4 data; // normal.xyz + distance.w
            /**
             * Normalizes the plane so that the normal has unit length and the
             * distance term is scaled accordingly.
             */
            void normalize();
        };

        /**
         * Extracts the six clipping planes of a frustum from a combined
         * projection-view matrix.
         *
         * The resulting planes are written to the provided array. Unless noted
         * otherwise, the order follows the engine convention:
         * left, right, bottom, top, near, far.
         *
         * @param planes Output array that will receive the six planes.
         *               Each plane is expressed as (nx, ny, nz, d).
         * @param matrix Projection-View matrix (clip-from-world).
         */
        static void extractPlanes(Plane planes[6], const float4x4& matrix);

    };

}