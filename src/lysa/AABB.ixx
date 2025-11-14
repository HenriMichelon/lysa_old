/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.aabb;

import lysa.math;
import lysa.types;

export namespace lysa {

    /**
     * Lightweight structure representing a 3D axis-aligned bounding box in the
     * engine's right-handed coordinate system.
     *
     * An AABB is defined by its minimum and maximum corners in object or world
     * space. It is commonly used for coarse collision detection, visibility
     * tests (frustum culling), and spatial queries.
     *
     * Notes:
     *  - The box is axis-aligned in whatever space its min/max are expressed.
     *  - Applying a general transform (with rotation/shear) to an AABB requires
     *    re-computing a new axis-aligned box that encloses the transformed
     *    oriented box; see toGlobal().
     */
    struct AABB {
        /**
         * Minimum corner of the box (component-wise). Typically corresponds to
         * the left/bottom/back corner: {min.x, min.y, min.z}.
         */
        float3 min{};
        /**
         * Maximum corner of the box (component-wise). Typically corresponds to
         * the right/top/front corner: {max.x, max.y, max.z}.
         */
        float3 max{};

        /**
         * Default-construct an empty/invalid AABB with zeroed corners.
         * The box becomes valid once min/max are assigned meaningful values.
         */
        AABB() = default;

        /**
         * Construct a bounding box from its minimum and maximum corners.
         *
         * @param min Minimum corner (x/y/z individually less-or-equal to max).
         * @param max Maximum corner (x/y/z individually greater-or-equal to min).
         */
        inline AABB(const float3& min, const float3& max) : min{min}, max{max} {}

        /**
         * Compute the world-space AABB that encloses this box transformed by the
         * given matrix.
         *
         * The input box is considered to be in local/object space. The result is
         * axis-aligned in the destination space of the transform (typically world
         * space). If the transform contains rotation or shear, the returned AABB
         * will be the minimal axis-aligned box that fully contains the rotated
         * (oriented) box.
         *
         * @param transform 4x4 row-major transform matrix (object -> world).
         * @return The transformed, axis-aligned bounding box.
         */
        AABB toGlobal(const float4x4& transform) const;
    };
}