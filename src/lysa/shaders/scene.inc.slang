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
cbuffer IndirectRootConstant : register(b0, space5) {
    uint instanceIndex;
};
#endif

struct VertexOutput {
    float4 position      : SV_POSITION;
    float3 worldPos      : POSITION;
    float3 normal        : NORMAL;
    float2 uv            : TEXCOORD0;
    float3 viewDirection : TEXCOORD1;
    float4 viewPos       : TEXCOORD2;
    nointerpolation uint materialIndex            : TEXCOORD3;
    nointerpolation uint meshSurfaceMaterialIndex : TEXCOORD4;
    float3x3 TBN         : TEXCOORD5;
};

struct Scene {
    float3   cameraPosition;
    float4x4 projection;
    float4x4 view;
    float4x4 viewInverse;
    float4   ambientLight;
    uint     lightsCount;
    uint     bloomEnabled;
    uint     ssaoEnabled;
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

float4 fetchColor(float2 uv, Material mat) {
    float4 color = mat.albedoColor;
    if (mat.diffuseTexture.index != -1) {
        color = textures[mat.diffuseTexture.index].Sample(
            samplers[mat.diffuseTexture.samplerIndex],
            uvTransform(mat.diffuseTexture, uv));
    }
    return color;
}

float3 fetchNormal(float2 uv, float3x3 TBN, float3 normal, Material mat) {
    if (mat.normalTexture.index != -1) {
        normal = textures[mat.normalTexture.index].Sample(
                     samplers[mat.normalTexture.samplerIndex],
                     uvTransform(mat.normalTexture, uv)).rgb;
        normal = normalize((normal * 2.0 - 1.0) * float3(mat.normalScale, mat.normalScale, 1.0f));
        normal = normalize(mul(normal, TBN));
    }
    return normal;
}

float fetchMetallic(float2 uv, Material mat) {
    return mat.metallicTexture.index == -1 ?
           mat.metallicFactor :
           mat.metallicFactor *
               textures[mat.metallicTexture.index].Sample(samplers[mat.metallicTexture.samplerIndex],
               uvTransform(mat.metallicTexture, uv)).b;
}

float fetchRoughness(float2 uv, Material mat) {
    return mat.roughnessTexture.index == -1 ?
           mat.roughnessFactor :
           mat.roughnessFactor *
               textures[mat.roughnessTexture.index].Sample(samplers[mat.roughnessTexture.samplerIndex],
               uvTransform(mat.roughnessTexture,
               uv)).g;
}

float3 fetchEmissiveColor(float2 uv, Material mat) {
    float3 emissiveColor = mat.emissiveFactor.rgb;
    if (mat.emissiveTexture.index != -1) {
        emissiveColor *= textures[mat.emissiveTexture.index].Sample(
           samplers[mat.emissiveTexture.samplerIndex], uvTransform(mat.emissiveTexture, uv)).rgb * mat.emissiveFactor.a;
    }
    return emissiveColor;
}