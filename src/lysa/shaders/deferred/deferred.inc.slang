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

[[vk::binding(0, 3)]] Texture2D positionBuffer  : register(t0, space3);
[[vk::binding(1, 3)]] Texture2D normalBuffer    : register(t1, space3);
[[vk::binding(2, 3)]] Texture2D albedoBuffer    : register(t2, space3);
[[vk::binding(3, 3)]] Texture2D emissiveBuffer  : register(t3, space3);
[[vk::binding(4, 3)]] Texture2D aoMap           : register(t4, space3);

float4 getColor(QuadOutput input) {
    float4 positionValue = positionBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 worldPos = positionValue.rgb;
    float viewposZ = positionValue.a;
    float4 normalValue = normalBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 normal = normalValue.rgb;
    float roughness = normalValue.a;
    float4 albedoValue = albedoBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 albedo = albedoValue.rgb;
    float metallic = albedoValue.a;
    float4 emissiveColorValue = emissiveBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 emissiveColor = emissiveColorValue.rgb;
    float3 viewDirection = normalize(scene.cameraPosition - worldPos);
    float ambientOcclusion = scene.ssaoEnabled == 1 ? aoMap.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).r : 1.0;
    //return float4(viewposZ, 0.0, 0.0, 1.0);
    return float4(
        calcLight(
            viewDirection,
            worldPos,
            albedo,
            normal,
            scene.ambientLight,
            ambientOcclusion,
            metallic,
            roughness,
            emissiveColor,
            viewposZ),
        1.0);
}
