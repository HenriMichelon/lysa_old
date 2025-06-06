/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

struct DrawIndirectCommand {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

[[vk::binding(0, 0)]] StructuredBuffer<uint> indices : register(t0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<MeshSurface> surfaces : register(t1, space0);
[[vk::binding(2, 0)]] RWStructuredBuffer<uint> input : register(u2, space0);
[[vk::binding(3, 0)]] RWStructuredBuffer<uint> surfaceCount : register(u3, space0);
[[vk::binding(4, 0)]] RWStructuredBuffer<Index> output : register(u4, space0);
[[vk::binding(5, 0)]] RWStructuredBuffer<DrawIndirectCommand> command : register(u5, space0);

[numthreads(1, 1, 1)]
void main() {
    uint vertexCount = 0;
    for (int s = 0; s < surfaceCount[0]; s++) {
        MeshSurface surface = surfaces[input[s]];
        uint count = min(2000*3, surface.indexCount);
        for (uint i = 0; i < count; i++) {
            Index index;
            index.index = indices[surface.firstIndex + i];
            index.surfaceIndex = surface.surfaceDataIndex;
            output[vertexCount + i] = index;
        }
        vertexCount += count;
    }
    command[0].vertexCount = vertexCount;
}