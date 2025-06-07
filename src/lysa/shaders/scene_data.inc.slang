/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
struct Model {
    float4x4 transform;
    float3   aabbMin;
    float   _pad0;
    float3   aabbMax;
    float   _pad1;
    uint     visible;
    float3  _pad2;
};

struct TextureInfo {
    int index;
    uint samplerIndex;
    float2 _pad0;
    float4x4 transform;
};

struct Material {
    float4 albedoColor;
    uint   pipelineId;
    int    transparency;
    float  alphaScissor;
    float  normalScale;
    TextureInfo diffuseTexture;
    TextureInfo normalTexture;
    float4 parameters[4];
}

struct MeshSurface {
    uint indexCount;
    uint indicesIndex;
    uint verticesIndex;
    uint materialIndex;
};

struct Instance {
    uint modelIndex;
    uint surfaceIndex;
}
