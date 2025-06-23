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

[[vk::binding(0, 3)]] Texture2D positionBuffer : register(t0, space3);
[[vk::binding(1, 3)]] Texture2D normalBuffer   : register(t1, space3);
[[vk::binding(2, 3)]] Texture2D albedoBuffer   : register(t2, space3);
[[vk::binding(3, 3)]] Texture2D emissiveBuffer : register(t3, space3);

float4 getColor(QuadOutput input) {
    float4 value = positionBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 worldPos = value.rgb;
    float metallic = value.a;
    float3 viewDirection = normalize(scene.cameraPosition - worldPos);

    value = normalBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 normal = value.rgb;
    float roughness = value.a;

    float3 albedo = albedoBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).rgb;

    value = emissiveBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 emissiveColor = value.rgb;
    bool ispbr = value.a == 1.0;

    return float4(
        calcLight(viewDirection, worldPos, albedo, normal, scene.ambientLight, ispbr, metallic, roughness, emissiveColor),
        1.0);
}
