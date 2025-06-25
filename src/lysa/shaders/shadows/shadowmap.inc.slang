/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "../resources.inc.slang"

struct Global {
    float3   lightPosition;
    float    farPlane;
    float4x4 lightSpace[6];
};

struct PushConstants {
    uint lightSpaceIndex;
};

[[vk::binding(0, 0)]] StructuredBuffer<Material> materials : register(t0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<MeshInstance> meshInstances : register(t1, space0);
[[vk::binding(2, 0)]] StructuredBuffer<Instance> instances : register(t2, space0);
[[vk::binding(3, 0)]] ConstantBuffer<Global> global  : register(b3, space0);
[[push_constant]] PushConstants pushConstants : register(b4, space0);
