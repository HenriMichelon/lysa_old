/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../scene.inc.slang"

struct QuadOutput {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

struct SSAOParams {
    float2 screenSize;
    float2 _pad0;
    float2 noiseScale; // screenSize / noiseTexSize
    float2 _pad1;
    float radius;
    float bias;
    float power;
    uint sampleCount;
    float4 samples[64];
};

[[vk::binding(0, 3)]] ConstantBuffer<SSAOParams> params : register(b0, space3);
[[vk::binding(1, 3)]] Texture2D positionBuffer : register(t1, space3);
[[vk::binding(2, 3)]] Texture2D normalBuffer : register(t2, space3);
[[vk::binding(3, 3)]] Texture2D noiseTexture : register(t3, space3);

float fragmentMain(QuadOutput input) : SV_TARGET {
    float4 positionValue = positionBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float3 worldPos = positionValue.rgb;
    float viewZ = positionValue.a;
    float3 viewPos = mul(scene.view, float4(worldPos, 1.0)).xyz;

    float3 normalWorld = normalBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).rgb;
    float3 normalView = normalize(mul(normalWorld, (float3x3)scene.view));
//return float4(normalView * 0.5 + 0.5, 1.0);
    float3 randomVec = noiseTexture.Sample(samplers[SAMPLER_NEAREST_NEAREST_REPEAT_REPEAT], input.uv * params.noiseScale).rgb;

    float3 tangent = normalize(randomVec - normalView * dot(randomVec, normalView));
    float3 bitangent = cross(normalView, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normalView);

    float occlusion = 0.0;
    for (uint i = 0; i < params.sampleCount; ++i) {
        float3 sampleOffset = mul(params.samples[i].rgb, TBN);
        float3 samplePos = viewPos + sampleOffset * params.radius;

        float4 offset = mul(scene.projection, float4(samplePos, 1.0));
        offset.xyz /= offset.w;
        float2 sampleUV = offset.xy * 0.5 + 0.5;
        sampleUV.y = 1.0 - sampleUV.y;

        float sampledViewZ = positionBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], sampleUV).a;
        float rangeCheck = smoothstep(0.0, 1.0, params.radius / abs(viewZ - sampledViewZ));
        float occlude = (sampledViewZ >= samplePos.z + params.bias ? 1.0 : 0.0);
        occlusion += occlude * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / params.sampleCount);
    occlusion = pow(occlusion, params.power);
    return occlusion;
}
