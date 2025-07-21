/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.aabb;

import lysa.math;

namespace lysa {

    // https://ktstephano.github.io/rendering/stratusgfx/aabbs
    AABB AABB::toGlobal(const float4x4& transform) const {
        // First extract the 8 transformed corners of the box using vmin/vmax
        const float3 corners[8] = {
            float3(mul(float4(min.x, min.y, min.z, 1.0f), transform).xyz),
            float3(mul(float4(min.x, max.y, min.z, 1.0f), transform).xyz),
            float3(mul(float4(min.x, min.y, max.z, 1.0f), transform).xyz),
            float3(mul(float4(min.x, max.y, max.z, 1.0f), transform).xyz),
            float3(mul(float4(max.x, min.y, min.z, 1.0f), transform).xyz),
            float3(mul(float4(max.x, max.y, min.z, 1.0f), transform).xyz),
            float3(mul(float4(max.x, min.y, max.z, 1.0f), transform).xyz),
            float3(mul(float4(max.x, max.y, max.z, 1.0f), transform).xyz)
        };

        // Now apply the min/max algorithm from before using the 8 transformed
        // corners
        auto newVmin = corners[0];
        auto newVmax = newVmin;

        // Start looping from corner 1 onwards
        for (auto i = 1; i < 8; ++i) {
            const auto& current = corners[i];
            newVmin = lysa::min(newVmin, current);
            newVmax = lysa::max(newVmax, current);
        }

        // Now pack them into our new bounding box
        return { newVmin, newVmax };
    }

}