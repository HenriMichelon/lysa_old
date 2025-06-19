/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"

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

[[vk::binding(2, 2)]] ConstantBuffer<Lights> lights : register(b2, space2);

float3 calcDirectionalLight(Light light, float3 worldPos, float3 normal, float shininess) {
    float3 L = -light.direction.xyz;
    float3 V = normalize(scene.cameraPosition - worldPos);
    float3 R = reflect(-L, normal);

    float diff = max(dot(normal, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), shininess);

    float3 diffuse = diff * light.color.rgb * light.color.w;
    float3 specular = spec * light.color.rgb * light.color.w;

    return diffuse + specular;
}
