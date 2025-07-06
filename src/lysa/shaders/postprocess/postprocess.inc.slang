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
    uint   applyBloom;
    float  time;
    float2 _pad0;
    uint2  imageSize;
};

static const int INPUT_BUFFER  = 0;
static const int DEPTH_BUFFER = 1;
static const int BLOOM_BUFFER = 2;

[[vk::binding(0, 0)]] ConstantBuffer<Params> params : register(b0, space0);
[[vk::binding(2, 0)]] Texture2D textures[] : register(t2, space0);
