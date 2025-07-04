/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
enum class Transparency {
    DISABLED = 0,
    ALPHA = 1,
//     SCISSOR = 2,
//     SCISSOR_ALPHA = 3,
};


struct Vertex {
    float4 position; // position + uv.x
    float4 normal;   // normal + uv.y
    float4 tangent;  // tangent + sign
};

struct MeshInstance {
    float4x4 transform;
    float3   aabbMin;
    float    _pad0;
    float3   aabbMax;
    float    _pad1;
    uint     visible;
    uint     castShadows;
    float2   _pad2;
};

struct TextureInfo {
    int      index;
    uint     samplerIndex;
    float2   _pad0;
    float4x4 transform;
};

struct Material {
    float4 albedoColor;

    uint   pipelineId;
    int    transparency;
    float  alphaScissor;
    float  normalScale;

    float  metallicFactor;
    float  roughnessFactor;
    float _pad1;
    float _pad2;

    float4 emissiveFactor;  // factor + strength

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

static const int LIGHT_DIRECTIONAL = 0;
static const int LIGHT_OMNI        = 1;
static const int LIGHT_SPOT        = 2;

struct Light {
    // light params
    int type; // Light::LightType
    float range;
    float cutOff;
    float outerCutOff;
    float4 position;
    float4 direction;
    float4 color; // RGB + Intensity;
    // shadow map params
    int mapIndex;
    uint cascadesCount;
	float2 _pad0;
    float4 cascadeSplitDepth;
    float4x4 lightSpace[6];
};