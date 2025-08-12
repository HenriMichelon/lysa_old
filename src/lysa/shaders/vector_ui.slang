/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "vector.inc.slang"
#include "samplers.inc.slang"

[[vk::binding(0, 0)]]  Texture2D textures[] : register(t0, space0);

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    float2 pos = 2 * (input.position.xy - 0.5); // remap to [-1,1]
    output.position = float4(pos.x, pos.y, 0, 1);
    output.uv = input.uv;
    output.color = input.color;
    output.textureIndex = input.textureIndex;
    return output;
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    if (input.textureIndex == -1) {
        return input.color;
    } else {
        input.uv.y = 1.0 - input.uv.y;
        float4 color = textures[input.textureIndex].Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR], input.uv);
        return float4(input.color.rgb * color.rgb, min(color.a, input.color.a));
    }
}
