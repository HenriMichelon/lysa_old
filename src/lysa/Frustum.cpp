/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.frustum;

import lysa.aabb;
import lysa.global;

namespace lysa {
    
    void Frustum::Plane::normalize() {
        const float invLen = 1.0f / length(normal);
        normal *= invLen;
        distance *= invLen;
    }

    void Frustum::extractPlanes(Plane planes[6], const float4x4& matrix) {
        // Gribb & Hartmann method
        // LEFT
        planes[0].normal.x = matrix[0][3] + matrix[0][0];
        planes[0].normal.y = matrix[1][3] + matrix[1][0];
        planes[0].normal.z = matrix[2][3] + matrix[2][0];
        planes[0].distance = matrix[3][3] + matrix[3][0];

        // RIGHT
        planes[1].normal.x = matrix[0][3] - matrix[0][0];
        planes[1].normal.y = matrix[1][3] - matrix[1][0];
        planes[1].normal.z = matrix[2][3] - matrix[2][0];
        planes[1].distance = matrix[3][3] - matrix[3][0];

        // BOTTOM
        planes[2].normal.x = matrix[0][3] + matrix[0][1];
        planes[2].normal.y = matrix[1][3] + matrix[1][1];
        planes[2].normal.z = matrix[2][3] + matrix[2][1];
        planes[2].distance = matrix[3][3] + matrix[3][1];

        // TOP
        planes[3].normal.x = matrix[0][3] - matrix[0][1];
        planes[3].normal.y = matrix[1][3] - matrix[1][1];
        planes[3].normal.z = matrix[2][3] - matrix[2][1];
        planes[3].distance = matrix[3][3] - matrix[3][1];

        // NEAR
        planes[4].normal.x = matrix[0][3] + matrix[0][2];
        planes[4].normal.y = matrix[1][3] + matrix[1][2];
        planes[4].normal.z = matrix[2][3] + matrix[2][2];
        planes[4].distance = matrix[3][3] + matrix[3][2];

        // FAR
        planes[5].normal.x = matrix[0][3] - matrix[0][2];
        planes[5].normal.y = matrix[1][3] - matrix[1][2];
        planes[5].normal.z = matrix[2][3] - matrix[2][2];
        planes[5].distance = matrix[3][3] - matrix[3][2];

        for (int i = 0; i < 6; i++) {
            planes[i].normalize();
        }
    }

    /*
     * https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    */

    // Frustum::Frustum(const float aspectRatio, const Node& node, const float fovY, const float zNear, const float zFar):
    //     Frustum(aspectRatio, node, node.getPositionGlobal(), fovY, zNear, zFar) {
    // }
    //
    // Frustum::Frustum(const float aspectRatio, const Node& node, const float3& position, const float fovY, const float zNear, const float zFar):
    //     Frustum(aspectRatio, position, node.getFrontVector(), node.getRightVector(), node.getUpVector(), fovY, zNear, zFar) {
    // }
    //
    // Frustum::Frustum(const float aspectRatio, const float3& position, const float3& front, const float3& right, const float3&up, float fovY, float zNear, float zFar) {
    //     const float halfVSide = zFar * tanf(radians(fovY) * .5f);
    //     const float halfHSide = halfVSide *  aspectRatio;
    //     const float3 frontMultFar = zFar * front;
    //
    //     nearFace = { position + zNear * front, front };
    //     farFace = { position + frontMultFar, -front };
    //     rightFace = { position,cross(frontMultFar - right * halfHSide, up) };
    //     leftFace = { position, cross(up,frontMultFar + right * halfHSide) };
    //     topFace = { position, cross(right, frontMultFar - up * halfVSide) };
    //     bottomFace = { position, cross(frontMultFar + up * halfVSide, right) };
    // }


}