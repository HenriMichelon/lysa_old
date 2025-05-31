/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "samplers.inc.slang"

struct Vertex {
    float3 position;
    float _pad0;
    float3 normal;
    float _pad1;
    float2 uv;
    float2 _pad2;
    float4 tangent;
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

struct Material {
    float4 albedoColor;
    float  shininess;
    int    diffuseTextureIndex;
    float _pad0;
    float _pad1;
    float4 parameters[4];
}

struct Index {
    uint index;
    uint surfaceIndex;
}

[[vk::binding(0, 0)]] StructuredBuffer<Vertex> vertices : register(t0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<Material> materials : register(t1, space0);
[[vk::binding(2, 0)]] Texture2D textures[200] : register(t2, space0); // Resources::MAX_TEXTURES

[[vk::binding(0, 2)]] ConstantBuffer<Scene> scene  : register(b0, space2);
[[vk::binding(1, 2)]] StructuredBuffer<InstanceData> instancesData : register(t1, space2);

[[vk::binding(0, 3)]] StructuredBuffer<Index> indices : register(t0, space3);

VertexOutput vertexMVP(InstanceData instanceData, Vertex input) {
    VertexOutput output;
    float4x4 model = instanceData.transform;

    float4 localPos = float4(input.position, 1.0);
    float4 worldPos = mul(model, localPos);
    output.worldPos = worldPos.xyz;

    output.position = mul(scene.projection, mul(scene.view, worldPos));

    float3 normalW = mul((float3x3)model, input.normal);
    output.normal = normalize(normalW);

    float3 tangentW = mul((float3x3)model, input.tangent.xyz);
    output.tangent = normalize(tangentW);
    output.bitangent = normalize(cross(normalW, tangentW));

    output.uv = input.uv;
    output.materialIndex = instanceData.materialIndex;
    return output;
}
