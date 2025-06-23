/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "resources.inc.slang"

struct Global {
    uint drawCommandsCount;
};

struct DrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int  vertexOffset;
    uint firstInstance;
};

struct DrawCommand {
    uint instanceIndex;
    DrawIndexedIndirectCommand command;
    float depth;
};

[[vk::binding(0, 0)]] ConstantBuffer<Global> global  : register(b0, space0);
[[vk::binding(1, 0)]] RWStructuredBuffer<DrawCommand> commands : register(u1, space0);


[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    uint i = id.x;
    if (i >= global.drawCommandsCount) return;

    for (uint pass = 0; pass < global.drawCommandsCount; ++pass) {
        bool isEven = (pass & 1) == 0;
        uint idx = 2 * i + (isEven ? 0 : 1);

        if (idx + 1 < global.drawCommandsCount) {
            DrawCommand a = commands[idx];
            DrawCommand b = commands[idx + 1];

            if (a.depth < b.depth) {
                commands[idx] = b;
                commands[idx + 1] = a;
            }
        }

        GroupMemoryBarrierWithGroupSync();
    }
}