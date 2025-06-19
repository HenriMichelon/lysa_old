/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "lighting.inc.slang"

struct QuadOutput {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

[[vk::binding(0, 3)]] Texture2D    positionBuffer   : register(t0, space3);
[[vk::binding(1, 3)]] Texture2D    normalBuffer     : register(t1, space3);
[[vk::binding(2, 3)]] Texture2D    albedoBuffer     : register(t2, space3);
// [[vk::binding(3, 3)]]  Texture2D    materialBuffer   : register(t3, space3);

float4 getColor(QuadOutput input) {
    float3 worldPos = positionBuffer.Sample(samplers[2], input.uv).rgb;
    float3 normal = normalBuffer.Sample(samplers[2], input.uv).rgb;
    float3 albedo = albedoBuffer.Sample(samplers[2], input.uv).rgb;
//     float2 material = materialBuffer.Sample(samplers[2], input.uv).rg;

    float3 ambient = scene.ambientLight.rgb * scene.ambientLight.w;
    float3 lit = 0.0f;
    for (uint i = 0; i < scene.lightsCount; i++) {
        lit += calcDirectionalLight(lights.light[i], worldPos, normal, 128.0);
    }

    return float4(albedo * (lit + ambient), 1.0);
}
