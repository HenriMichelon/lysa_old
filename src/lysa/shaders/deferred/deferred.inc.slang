/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../lighting/lighting.inc.slang"

struct QuadOutput {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

[[vk::binding(0, 3)]] Texture2D positionBuffer : register(t0, space3);
[[vk::binding(1, 3)]] Texture2D normalBuffer   : register(t1, space3);
[[vk::binding(2, 3)]] Texture2D albedoBuffer   : register(t2, space3);
[[vk::binding(3, 3)]] Texture2D emissiveBuffer : register(t3, space3);

float4 getColor(QuadOutput input) {
    float4 positionValue = positionBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 worldPos = positionValue.rgb;
    float metallic = positionValue.a;
    float4 normalValue = normalBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 normal = normalValue.rgb;
    float roughness = normalValue.a;
    float3 albedo = albedoBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).rgb;
    float3 emissiveColor = emissiveBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).rgb;
    float3 viewDirection = normalize(scene.cameraPosition - worldPos);
//     return float4(albedo, 1.0);
    return float4(
        calcLight(viewDirection, worldPos, albedo, normal, scene.ambientLight, metallic, roughness, emissiveColor),
        1.0);
}
