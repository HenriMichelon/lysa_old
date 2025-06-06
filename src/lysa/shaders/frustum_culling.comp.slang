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
        return dot(normal, point) + distance;
    }
};

struct Global {
    uint pipelineId;
    uint surfaceCount;
    Plane planes[6];
};

struct DrawIndirectCommand {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

[[vk::binding(0, 0)]] ConstantBuffer<Global> global  : register(b0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<uint> indices : register(t1, space0);
[[vk::binding(2, 0)]] StructuredBuffer<Model> models : register(t2, space0);
[[vk::binding(3, 0)]] StructuredBuffer<Material> materials : register(t3, space0);
[[vk::binding(4, 0)]] StructuredBuffer<MeshSurface> surfaces : register(t4, space0);
[[vk::binding(5, 0)]] RWStructuredBuffer<Index> output : register(u5, space0);
[[vk::binding(6, 0)]] RWStructuredBuffer<DrawIndirectCommand> command : register(u6, space0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    if (id.x >= global.surfaceCount) {
        return;
    }

    MeshSurface surface = surfaces[id.x];
    if (materials[surface.materialIndex].pipelineId != global.pipelineId) {
        return;
    }
    Model model = models[surface.modelIndex];

    [unroll]
    for (int i = 0; i < 6; ++i) {
        Plane plane = global.planes[i];
        float3 positiveVertex = float3(
            (plane.normal.x >= 0.0f) ? model.aabbMax.x : model.aabbMin.x,
            (plane.normal.y >= 0.0f) ? model.aabbMax.y : model.aabbMin.y,
            (plane.normal.z >= 0.0f) ? model.aabbMax.z : model.aabbMin.z
        );
        if (plane.signedDistance(positiveVertex) < 0.0f) {
            return;
        }
    }

    uint offset;
    InterlockedAdd(command[0].vertexCount, surface.indexCount, offset);
    for (uint i = 0; i < surface.indexCount; i++) {
        uint idx = offset + i;
        output[idx].index = indices[surface.firstIndex + i];
        output[idx].surfaceIndex = surface.surfaceDataIndex;
    }

}