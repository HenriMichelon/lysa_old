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
    SamplerState sampler = samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR];
    float3 lumaWeight = float3(0.299, 0.587, 0.114);

    float lumaC  = dot(inputImage.Sample(sampler, input.uv).rgb, lumaWeight);
    float lumaN  = dot(inputImage.Sample(sampler, input.uv + float2(0, -1) / params.imageSize).rgb, lumaWeight);
    float lumaS  = dot(inputImage.Sample(sampler, input.uv + float2(0, 1) / params.imageSize).rgb, lumaWeight);
    float lumaW  = dot(inputImage.Sample(sampler, input.uv + float2(-1, 0) / params.imageSize).rgb, lumaWeight);
    float lumaE  = dot(inputImage.Sample(sampler, input.uv + float2(1, 0) / params.imageSize).rgb, lumaWeight);

    float edgeH = abs(lumaW - lumaC) + abs(lumaC - lumaE);
    float edgeV = abs(lumaN - lumaC) + abs(lumaC - lumaS);

    bool isEdge = edgeH > data.edgeThreshold || edgeV > data.edgeThreshold;
    return isEdge ? float4(edgeH, edgeV, 0, 0) : float4(0, 0, 0, 0);
    //return float4(edgeH, edgeV, 0, 0);
}
