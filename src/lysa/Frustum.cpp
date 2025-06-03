/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.frustum_culling;

import lysa.aabb;
import lysa.global;

namespace lysa {
    /*
     * https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    */

    Frustum::Frustum(const float aspectRatio, const std::shared_ptr<Node>& node, const float fovY, const float zNear, const float zFar):
        Frustum(aspectRatio, node, node->getPositionGlobal(), fovY, zNear, zFar) {
    }

    Frustum::Frustum(const float aspectRatio, const std::shared_ptr<Node>& node, const float3& position, const float fovY, const float zNear, const float zFar):
        Frustum(aspectRatio, position, node->getFrontVector(), node->getRightVector(), node->getUpVector(), fovY, zNear, zFar) {
    }

    Frustum::Frustum(const float aspectRatio, const float3& position, const float3& front, const float3& right, const float3&up, float fovY, float zNear, float zFar) {
        const float halfVSide = zFar * tanf(radians(fovY) * .5f);
        const float halfHSide = halfVSide *  aspectRatio;
        const float3 frontMultFar = zFar * front;

        nearFace = { position + zNear * front, front };
        farFace = { position + frontMultFar, -front };
        rightFace = { position,cross(frontMultFar - right * halfHSide, up) };
        leftFace = { position, cross(up,frontMultFar + right * halfHSide) };
        topFace = { position, cross(right, frontMultFar - up * halfVSide) };
        bottomFace = { position, cross(frontMultFar + up * halfVSide, right) };
    }


}