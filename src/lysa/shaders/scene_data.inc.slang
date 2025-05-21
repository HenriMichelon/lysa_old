/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
struct Vertex {
    float3 position;
    float3 normal;
    float2 uv;
    float3 tangent;
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
};

struct Material {
    float4 albedoColor;
    float  shininess;
    int    diffuseTextureIndex;
}

struct Index {
    uint index;
    uint surfaceIndex;
}

