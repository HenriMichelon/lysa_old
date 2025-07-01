/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "shadowmap.inc.slang"

struct VertexInput {
    float4 position : POSITION; // position + uv.x
#ifdef __SPIRV__
    uint instanceId : SV_StartInstanceLocation;
    #define instanceIndex input.instanceId
#endif
}

#ifndef __SPIRV__
cbuffer IndirectRootConstant : register(b0, space4) {
    uint instanceIndex;
};
#endif

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 worldPos : TEXCOORD1;
    nointerpolation uint materialIndex : TEXCOORD2;
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

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    Instance instance = instances[instanceIndex];
    float4x4 model = meshInstances[instance.meshInstanceIndex].transform;
    Material mat = materials[instance.materialIndex];
    float4 positionW = mul(model, float4(input.position.xyz, 1.0));
    output.worldPos = positionW;
    output.position = mul(global.lightSpace, positionW);
    output.materialIndex = instance.materialIndex;
    return output;
}

float fragmentMain(VertexOutput input) : SV_Depth {
    Material mat = materials[input.materialIndex];
    float4 color = fetchColor(input, mat);
    if (mat.transparency != Transparency.DISABLED && color.a < 0.75) {
       discard;
    }
    return length(input.worldPos.xyz - global.lightPosition.xyz) / global.lightPosition.w;
}