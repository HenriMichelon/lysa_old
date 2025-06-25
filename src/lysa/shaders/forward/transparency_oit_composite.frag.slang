/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../samplers.inc.slang"

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

[[vk::binding(0, 3)]] Texture2D accumBuffer  : register(t0, space3);
[[vk::binding(1, 3)]] Texture2D revealBuffer : register(t1, space3);

// https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
bool isApproximatelyEqual(float a, float b) {
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * 0.00001f;
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    float revealage = revealBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).r;
    if (isApproximatelyEqual(revealage, 1.0f)) {
        discard;
    }
    float4 accum = accumBuffer.Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    return float4(accum.rgb / max(accum.a, 0.00001f), revealage);
}
