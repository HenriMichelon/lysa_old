/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.frustum;

import lysa.aabb;
import lysa.types;

namespace lysa {
    
    void Frustum::Plane::normalize() {
        float3 n = data.xyz;
        const float len = length(n);
        n /= len;
        data = { n.x, n.y, n.z, data.w / len};
    }

    void Frustum::extractPlanes(Plane planes[6], const float4x4& matrix) {
        // Gribb & Hartmann method
        // LEFT
        planes[0].data = {
            matrix[0][3] + matrix[0][0],
            matrix[1][3] + matrix[1][0],
            matrix[2][3] + matrix[2][0],
            matrix[3][3] + matrix[3][0] };

        // RIGHT
        planes[1].data  = {
            matrix[0][3] - matrix[0][0],
            matrix[1][3] - matrix[1][0],
            matrix[2][3] - matrix[2][0],
            matrix[3][3] - matrix[3][0] };

        // BOTTOM
        planes[2].data = {
            matrix[0][3] + matrix[0][1],
            matrix[1][3] + matrix[1][1],
            matrix[2][3] + matrix[2][1],
            matrix[3][3] + matrix[3][1] };

        // TOP
        planes[3].data = {
            matrix[0][3] - matrix[0][1],
            matrix[1][3] - matrix[1][1],
            matrix[2][3] - matrix[2][1],
            matrix[3][3] - matrix[3][1] };

        // NEAR
        planes[4].data = {
            matrix[0][3] + matrix[0][2],
            matrix[1][3] + matrix[1][2],
            matrix[2][3] + matrix[2][2],
            matrix[3][3] + matrix[3][2] };

        // FAR
        planes[5].data = {
            matrix[0][3] - matrix[0][2],
            matrix[1][3] - matrix[1][2],
            matrix[2][3] - matrix[2][2],
            matrix[3][3] - matrix[3][2] };

        for (int i = 0; i < 6; i++) {
            planes[i].normalize();
        }
    }

}