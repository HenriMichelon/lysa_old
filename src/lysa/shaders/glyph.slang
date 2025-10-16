/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "vector.inc.slang"
#include "glyph.inc.slang"
#include "samplers.inc.slang"

[[vk::binding(0, 0)]] ConstantBuffer<GlobalUniform> global  : register(b0, space0);
[[vk::binding(1, 0)]] Texture2D textures[] : register(t1, space0);
[[vk::binding(0, 2)]] ConstantBuffer<Fonts> fonts : register(t0, space2);

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    output.position = mul(global.projection, mul(global.view,  float4(input.position, 1.0)));
    output.color = input.color;
    output.uv = input.uv;
    output.textureIndex = input.textureIndex;
    output.fontIndex = input.fontIndex;
    return output;
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    return fragmentGlyph(input);
}
