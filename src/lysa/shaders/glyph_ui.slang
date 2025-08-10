/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "vector.inc.slang"
#include "samplers.inc.slang"

[[vk::binding(0, 0)]] Texture2D textures[] : register(t0, space0);

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    float2 pos = 2 * (input.position.xy - 0.5); // remap to [-1,1]
    output.position = float4(pos.x, -pos.y, 0, 1);
    output.uv = input.uv;
    output.textureIndex = input.textureIndex;
    return output;
}

float median(float r, float g, float b) {
    return max(min(r,g), min(max(r,g), b));
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    input.uv.y = 1.0 - input.uv.y;
    float4 msdf = textures[input.textureIndex].Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR], input.uv);
    float sd = median(msdf.r, msdf.g, msdf.b) - 0.5;
    float px = 4.0 * fwidth(input.uv.x * 612);
    float alpha = saturate(sd * (1.0 / px) + 0.5);
    float4 textColor = (0.5, 0.5, 0.5, 1.0);
    return float4(textColor.rgb, textColor.a * alpha);
}