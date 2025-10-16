/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "vector.inc.slang"
#include "glyph.inc.slang"
#include "samplers.inc.slang"

[[vk::binding(0, 0)]] Texture2D textures[] : register(t0, space0);
[[vk::binding(0, 2)]] ConstantBuffer<Fonts> fonts : register(t0, space2);

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    float2 pos = 2 * (input.position.xy - 0.5); // remap to [-1,1]
    output.position = float4(pos.x, pos.y, 0, 1);
    output.color = input.color;
    output.uv = input.uv;
    output.textureIndex = input.textureIndex;
    output.fontIndex = input.fontIndex;
    return output;
}


float4 fragmentMain(VertexOutput input) : SV_TARGET {
    return fragmentGlyph(input);
}