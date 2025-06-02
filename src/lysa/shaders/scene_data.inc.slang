/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "samplers.inc.slang"

struct Vertex {
    float4 position; // position + uv.x
    float4 normal;   // normal + uv.y
    float4 tangent;  // tangent + sign
};

struct VertexOutput {
    float4 position      : SV_POSITION;
    float3 worldPos      : TEXCOORD0;
    float3 normal        : TEXCOORD1;
    float2 uv            : TEXCOORD2;
    float3 tangent       : TEXCOORD3;
    float3 bitangent     : TEXCOORD4;
    nointerpolation uint materialIndex : TEXCOORD5;
};

struct Scene {
    float3   cameraPosition;
    float4x4 projection;
    float4x4 view;
    float4x4 viewInverse;
    float4   ambientLight;
}

struct InstanceData {
    float4x4 transform;
    uint firstVertex;
    uint materialIndex;
    float _pad0;
    float _pad1;
};

#define TRANSPARENCY_DISABLED      0
#define TRANSPARENCY_ALPHA         1
#define TRANSPARENCY_SCISSOR       2
#define TRANSPARENCY_SCISSOR_ALPHA 3

struct TextureInfo {
    int index;
    uint samplerIndex;
    float2 _pad0;
    float4x4 transform;
};

struct Material {
    float4 albedoColor;
    int    transparency;
    float  alphaScissor;
    float  normalScale;
    float  _pad0;
    TextureInfo diffuseTexture;
    TextureInfo normalTexture;
    float4 parameters[4];
}

struct Index {
    uint index;
    uint surfaceIndex;
}

// Apply texture UV transforms
float2 uvTransform(const TextureInfo texture, const float2 UV) {
    return mul(float3x3(texture.transform), float3(UV, 1)).xy;
}

[[vk::binding(0, 0)]] StructuredBuffer<Vertex> vertices : register(t0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<Material> materials : register(t1, space0);
[[vk::binding(2, 0)]] Texture2D textures[500] : register(t2, space0); // Resources::MAX_TEXTURES

[[vk::binding(0, 2)]] ConstantBuffer<Scene> scene  : register(b0, space2);
[[vk::binding(1, 2)]] StructuredBuffer<InstanceData> instancesData : register(t1, space2);

[[vk::binding(0, 3)]] StructuredBuffer<Index> indices : register(t0, space3);

VertexOutput vertexMVP(InstanceData instanceData, Vertex input) {
    VertexOutput output;
    float4x4 model = instanceData.transform;

    float4 position = float4(input.position.xyz, 1.0);
    float4 positionW = mul(model, position);
    output.worldPos = positionW.xyz;

    output.position = mul(scene.projection, mul(scene.view, positionW));

    float3 normalW = mul((float3x3)model, input.normal.xyz);
    output.normal = normalize(normalW);

    float3 tangentW = mul((float3x3)model, input.tangent.xyz);
    output.tangent = normalize(tangentW);
    output.bitangent = normalize(cross(normalW, tangentW));

    output.uv = float2(input.position.w, input.normal.w);
    output.materialIndex = instanceData.materialIndex;
    return output;
}
