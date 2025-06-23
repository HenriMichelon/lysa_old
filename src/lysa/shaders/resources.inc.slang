/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
struct Vertex {
    float4 position; // position + uv.x
    float4 normal;   // normal + uv.y
    float4 tangent;  // tangent + sign
};

struct MeshInstance {
    float4x4 transform;
    float3   aabbMin;
    float   _pad0;
    float3   aabbMax;
    float   _pad1;
    uint     visible;
    float3  _pad2;
};

struct TextureInfo {
    int      index;
    uint     samplerIndex;
    float2   _pad0;
    float4x4 transform;
};

struct Material {
    float4 albedoColor;
    int    transparency;
    float  alphaScissor;
    float  metallicFactor; // -1.0f -> non PBR material
    float  roughnessFactor;
    float3 emissiveFactor;
    float  emissiveStrength;
    float  normalScale;
    uint   pipelineId;
    float2 _pad0;
    TextureInfo diffuseTexture;
    TextureInfo normalTexture;
    TextureInfo metallicTexture;
    TextureInfo roughnessTexture;
    TextureInfo emissiveTexture;
    float4 parameters[4];
}

struct MeshSurface {
    uint indexCount;
    uint indicesIndex;
    uint verticesIndex;
};

struct Instance {
    uint  meshInstanceIndex;
    uint  meshSurfaceIndex;
    uint  materialIndex; // can be overridden in MeshInstance
    uint  meshSurfaceMaterialIndex; // original when overridden
}
