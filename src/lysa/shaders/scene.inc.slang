/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "samplers.inc.slang"
#include "scene_data.inc.slang"

#define TRANSPARENCY_DISABLED      0
#define TRANSPARENCY_ALPHA         1
#define TRANSPARENCY_SCISSOR       2
#define TRANSPARENCY_SCISSOR_ALPHA 3

struct VertexInput {
    [[vk::location(0)]] float4 position : POSITION; // position + uv.x
    [[vk::location(1)]] float4 normal   : NORMAL;   // normal + uv.y
    [[vk::location(2)]] float4 tangent  : TANGENT;  // tangent + sign
    uint instanceId : SV_InstanceID;
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
    uint     lightsCount;
}

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
    float farPlane;
    uint cascadesCount;
	float _pad0;
    float4 cascadeSplitDepth;
    float4x4 lightSpace[6];
};

struct Lights {
    Light[100] light;
}

// Apply texture UV transforms
float2 uvTransform(const TextureInfo texture, const float2 UV) {
    return mul(float3x3(texture.transform), float3(UV, 1)).xy;
}

[[vk::binding(0, 0)]] StructuredBuffer<Material> materials : register(t0, space0);
[[vk::binding(1, 0)]] StructuredBuffer<MeshSurface> surfaces : register(t1, space0);
[[vk::binding(2, 0)]] Texture2D textures[] : register(t2, space0);

[[vk::binding(0, 2)]] ConstantBuffer<Scene> scene  : register(b0, space2);
[[vk::binding(1, 2)]] StructuredBuffer<Model> models : register(t1, space2);
[[vk::binding(2, 2)]] ConstantBuffer<Lights> lights : register(b2, space2);

[[vk::binding(0, 3)]] StructuredBuffer<Instance> instances : register(t0, space3);
