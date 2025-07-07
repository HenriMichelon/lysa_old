/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "postprocess.inc.slang"

struct Data {
    float edgeThreshold;
    int   blendMaxSteps;
};

[[vk::binding(1, 0)]] ConstantBuffer<Data> data : register(b1, space0);

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    Texture2D inputImage = textures[INPUT_BUFFER];
    Texture2D blendBuffer = textures[BLOOM_BUFFER];
    SamplerState sampler = samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR];

    float4 color = inputImage.Sample(sampler, input.uv);
    float2 blend = blendBuffer.Sample(sampler, input.uv).rg;
    float4 n = inputImage.Sample(sampler, input.uv + float2(0, -1) / params.imageSize);
    float4 e = inputImage.Sample(sampler, input.uv + float2(1, 0) / params.imageSize);

    /*float4 blended = lerp(color, n, blend.r);
    blended = lerp(blended, e, blend.g);*/
    float4 blended = color;
    for (int i = 1; i <= data.blendMaxSteps; ++i) {
        float2 offset = float2(i, 0) / params.imageSize;
        float4 sampleE = inputImage.Sample(sampler, input.uv + offset);
        blended = lerp(blended, sampleE, blend.g * 0.5); // pondère avec le blend
    }
    return blended;
}
