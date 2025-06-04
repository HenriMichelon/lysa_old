/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

struct Plane {
    float3 normal;
    float  distance;
    float signedDistance(float3 point) {
        return dot(normal, point) - distance;
    }
};

struct Global {
    uint  pipelineId;
    uint  surfacesCount;
    Plane planes[6];
};

[[vk::binding(0, 0)]] ConstantBuffer<Global> global  : register(b0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<Model> models : register(t1, space0);
[[vk::binding(2, 0)]] StructuredBuffer<MeshSurface> surfaces : register(t2, space0);

[[vk::binding(0, 1)]] AppendStructuredBuffer<Index> indices : register(t0, space1);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    uint index = id.x;
    if (index >= global.surfacesCount) {
        return;
    }
//
//     AABB box = aabbBuffer[index];
//
//     [unroll]
//     for (int i = 0; i < 6; ++i) {
//         Plane plane = global.planes[i];
//
//         float3 positiveVertex = float3(
//             (plane.normal.x >= 0.0f) ? box.max.x : box.min.x,
//             (plane.normal.y >= 0.0f) ? box.max.y : box.min.y,
//             (plane.normal.z >= 0.0f) ? box.max.z : box.min.z
//         );
//
//         if (plane.signedDistance(positiveVertex) < 0.0f) {
//             return;
//         }
//     }
//
//     visibleIndices.Append(index);
}