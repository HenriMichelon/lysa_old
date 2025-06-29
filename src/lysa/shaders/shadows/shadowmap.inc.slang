/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "../resources.inc.slang"

struct Global {
    float4x4 lightSpace;
    float4   lightPosition;  // XYZ: Position, W: far plane
}

[[vk::binding(0, 0)]] StructuredBuffer<Material> materials : register(t0, space0);

[[vk::binding(1, 1)]] StructuredBuffer<MeshInstance> meshInstances : register(t1, space1);

[[vk::binding(0, 2)]] StructuredBuffer<Instance> instances : register(t0, space2);

[[vk::binding(0, 3)]] ConstantBuffer<Global> global : register(b0, space3);
