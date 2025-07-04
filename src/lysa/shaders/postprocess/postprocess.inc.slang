/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../samplers.inc.slang"

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

struct Params {
    uint2 imageSize;
    float time;
};

[[vk::binding(0, 0)]] ConstantBuffer<Params> params : register(b0, space0);

[[vk::binding(0, 2)]] Texture2D inputImage  : register(t0, space2);
[[vk::binding(1, 2)]] Texture2D depthBuffer : register(t1, space2);
