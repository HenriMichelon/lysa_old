/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "resources.inc.slang"

struct Plane {
    float3 normal;
    float  distance;
    float signedDistance(float3 point) {
        return dot(normal, point) + distance;
    }
};

struct Global {
    uint drawCommandsCount;
    Plane planes[6];
    float4x4 viewMatrix;
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
};

[[vk::binding(0, 0)]] ConstantBuffer<Global> global  : register(b0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<MeshInstance> meshInstances : register(t1, space0);
[[vk::binding(2, 0)]] StructuredBuffer<Instance> instances : register(t2, space0);
[[vk::binding(3, 0)]] StructuredBuffer<DrawCommand> input : register(t3, space0);
[[vk::binding(4, 0)]] AppendStructuredBuffer<DrawCommand> output : register(u4, space0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    if (id.x >= global.drawCommandsCount) {
        return;
    }

    DrawCommand command = input[id.x];
    Instance instance = instances[command.instanceIndex];
    MeshInstance meshInstance = meshInstances[instance.meshInstanceIndex];
    if (meshInstance.visible == 0 || meshInstance.castShadows == 0) {
        return;
    }

    [unroll]
    for (int i = 0; i < 6; ++i) {
        Plane plane = global.planes[i];
        float3 positiveVertex = float3(
            (plane.normal.x >= 0.0f) ? meshInstance.aabbMax.x : meshInstance.aabbMin.x,
            (plane.normal.y >= 0.0f) ? meshInstance.aabbMax.y : meshInstance.aabbMin.y,
            (plane.normal.z >= 0.0f) ? meshInstance.aabbMax.z : meshInstance.aabbMin.z
        );
        if (plane.signedDistance(positiveVertex) < -0.0f) {
            return;
        }
    }

    output.Append(command);
}