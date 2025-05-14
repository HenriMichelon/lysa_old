/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.aabb;

import lysa.global;

export namespace lysa {

    /**
    * %A 3D axis-aligned bounding box.
    */
    struct AABB {
        //! leftmost corner
        float3 min{};
        //! rightmost corner
        float3 max{};

        /**
         * Creates an empty bounding box
         */
        AABB() = default;

        /**
         * Creates a bounding box
         */
        inline AABB(const float3& min, const float3& max) : min{min}, max{max} {}

        /**
         * Apply the given transform to the bounding box
         */
        AABB toGlobal(const float4x4& transform) const;
    };
}