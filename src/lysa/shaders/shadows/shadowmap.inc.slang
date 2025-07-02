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
    float    transparencyScissor;
    float    transparencyColorScissor;
}

[[vk::binding(0, 0)]] StructuredBuffer<Material> materials : register(t0, space0);
[[vk::binding(2, 0)]] Texture2D textures[] : register(t2, space0);

[[vk::binding(1, 1)]] StructuredBuffer<MeshInstance> meshInstances : register(t1, space1);

[[vk::binding(0, 2)]] StructuredBuffer<Instance> instances : register(t0, space2);

[[vk::binding(0, 3)]] ConstantBuffer<Global> global : register(b0, space3);

[[vk::binding(0, 4)]] SamplerState samplers[20] : register(s0, space4);


struct VertexInput {
    float4 position : POSITION; // position + uv.x
    float4 normal : NORMAL; // normal + uv.y
#ifdef __SPIRV__
    uint instanceId : SV_StartInstanceLocation;
    #define instanceIndex input.instanceId
#endif
}

#ifndef __SPIRV__
cbuffer IndirectRootConstant : register(b0, space5) {
    uint instanceIndex;
};
#endif

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 worldPos : TEXCOORD1;
    nointerpolation uint materialIndex : TEXCOORD2;
}

struct FragmentOutput {
    float depth : SV_Depth;
    float4 transparencyColor : SV_Target; // packed RGB + alpha
}

// Apply texture UV transforms
float2 uvTransform(const TextureInfo texture, const float2 UV) {
    return mul(float3x3(texture.transform), float3(UV, 1)).xy;
}

float4 fetchColor(VertexOutput input, Material mat) {
    float4 color = mat.albedoColor;
    if (mat.diffuseTexture.index != -1) {
        color = textures[mat.diffuseTexture.index].Sample(
            samplers[mat.diffuseTexture.samplerIndex],
            uvTransform(mat.diffuseTexture, input.uv));
    }
    return color;
}