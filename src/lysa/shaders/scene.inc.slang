/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "samplers.inc.slang"
#include "resources.inc.slang"

struct VertexInput {
    float4 position : POSITION; // position + uv.x
    float4 normal   : NORMAL;   // normal + uv.y
    float4 tangent  : TANGENT;  // tangent + sign
#ifdef __SPIRV__
    uint instanceId : SV_StartInstanceLocation;
    #define instanceIndex input.instanceId
#endif
};

#ifndef __SPIRV__
cbuffer IndirectRootConstant : register(b0, space4) {
    uint instanceIndex;
};
#endif

struct VertexOutput {
    float4 position      : SV_POSITION;
    float3 worldPos      : TEXCOORD0;
    float3 normal        : TEXCOORD1;
    float2 uv            : TEXCOORD2;
    float3 viewDirection : TEXCOORD3;
    nointerpolation uint materialIndex            : TEXCOORD4;
    nointerpolation uint meshSurfaceMaterialIndex : TEXCOORD5;
    float3x3 TBN         : TEXCOORD6;
};

struct Scene {
    float3   cameraPosition;
    float4x4 projection;
    float4x4 view;
    float4x4 viewInverse;
    float4   ambientLight;
    uint     lightsCount;
}

struct Lights {
    Light[100] light;
}

// Apply texture UV transforms
float2 uvTransform(const TextureInfo texture, const float2 UV) {
    return mul(float3x3(texture.transform), float3(UV, 1)).xy;
}

// Converts a color from sRGB gamma to linear light gamma
float4 toLinear(float4 sRGB) {
    float3 cutoff = step(float3(0.04045), sRGB.rgb);
    float3 higher = pow((sRGB.rgb + float3(0.055)) / float3(1.055), float3(2.));
    float3 lower  = sRGB.rgb / float3(12.92);
    float3 linearRGB = lerp(lower, higher, cutoff);
    return float4(linearRGB, sRGB.a);
}

[[vk::binding(0, 0)]] StructuredBuffer<Material> materials : register(t0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<MeshSurface> meshSurfaces : register(t1, space0);
[[vk::binding(2, 0)]] Texture2D textures[] : register(t2, space0);

[[vk::binding(0, 2)]] ConstantBuffer<Scene> scene  : register(b0, space2);
[[vk::binding(1, 2)]] StructuredBuffer<MeshInstance> meshInstances : register(t1, space2);
[[vk::binding(2, 2)]] ConstantBuffer<Lights> lights : register(b2, space2);
[[vk::binding(3, 2)]] Texture2D shadowMaps[] : register(t3, space2);

float4 fetchColor(VertexOutput input, Material mat) {
    float4 color = mat.albedoColor;
    if (mat.diffuseTexture.index != -1) {
        color = textures[mat.diffuseTexture.index].Sample(
            samplers[mat.diffuseTexture.samplerIndex],
            uvTransform(mat.diffuseTexture, input.uv));
    }
    return color;
}

float3 fetchNormal(VertexOutput input, Material mat) {
    float3 normal;
    if (mat.normalTexture.index != -1) {
        normal = textures[mat.normalTexture.index].Sample(
                     samplers[mat.normalTexture.samplerIndex],
                     uvTransform(mat.normalTexture, input.uv)).rgb;
        normal = normalize((normal * 2.0 - 1.0) * float3(mat.normalScale, mat.normalScale, 1.0f));
        normal = normalize(mul(normal, input.TBN));
    } else
    {
        normal = input.normal;
    }
    return normal;
}

float fetchMetallic(VertexOutput input, Material mat) {
    return mat.metallicTexture.index == -1 ?
           mat.metallicFactor :
           mat.metallicFactor *
               textures[mat.metallicTexture.index].Sample(samplers[mat.metallicTexture.samplerIndex],
               uvTransform(mat.metallicTexture, input.uv)).b;
}

float fetchRoughness(VertexOutput input, Material mat) {
    return mat.roughnessTexture.index == -1 ?
           mat.roughnessFactor :
           mat.roughnessFactor *
               textures[mat.roughnessTexture.index].Sample(samplers[mat.roughnessTexture.samplerIndex],
               uvTransform(mat.roughnessTexture, input.uv)).g;
}

float3 fetchEmissiveColor(VertexOutput input, Material mat) {
    float3 emissiveColor = mat.emissiveFactor;
    if (mat.emissiveTexture.index != -1) {
        emissiveColor *= toLinear(textures[mat.emissiveTexture.index].Sample(
           samplers[mat.emissiveTexture.samplerIndex], uvTransform(mat.emissiveTexture, input.uv))).rgb;
    }
    return emissiveColor;
}