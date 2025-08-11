/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "vector.inc.slang"
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

// https://www.redblobgames.com/x/2404-distance-field-effects/

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(VertexOutput input, FontParams font) {
  float2 screenTexSize =  float2(1.0) / fwidth(input.uv);
  return max(0.5 * dot(font.pxRange, screenTexSize), 1.0);
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    FontParams font = fonts.font[input.fontIndex];
    input.uv.y = 1.0 - input.uv.y;
    float4 distances = textures[input.textureIndex].Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR], input.uv);

    // distances are stored with 1.0 meaning "inside" and 0.0 meaning "outside"
    float d_msdf = median(distances.r, distances.g, distances.b);
    float d_sdf = distances.a; // mtsdf format only
    d_msdf = min(d_msdf, d_sdf + 0.1);  // HACK: to fix glitch in msdf near edges

    // blend between sharp and rounded corners
    float d_inner = lerp(d_msdf, d_sdf, 0.0);
    float d_outer = lerp(d_msdf, d_sdf, 0.0);

    // typically 0.5 is the threshold, >0.5 inside <0.5 outside
    float inverted_threshold = 1.0 - font.threshold; // because I want the ui to be +larger -smaller
    float width = screenPxRange(input, font);
    float inner = width * (d_inner - inverted_threshold) + 0.5 + font.outlineBias;
    float outer = width * (d_outer - inverted_threshold + font.outlineWidthRelative) + 0.5 + font.outlineBias + font.outlineWidthAbsolute;

    float inner_opacity = clamp(inner, 0.0, 1.0);
    float4 inner_color = input.color;
    float outer_opacity = clamp(outer, 0.0, 1.0);
    float4 outer_color = float4(0, 0, 0, 1); // TODO put in uniform

    if (font.outlineBlur > 0.0) {
        // NOTE: the smoothstep fails when the two edges are the same, and I wish it
        // would act like a step function instead of failing.
        // NOTE: I'm using d_sdf here because I want the shadows to be rounded
        // even when outlines are sharp. But I don't yet have implemented a way
        // to see the sharp outline with a rounded shadow.
        float blur_start = font.outlineWidthRelative + font.outlineWidthAbsolute / width;
        outer_color.a = smoothstep(blur_start,
                                   blur_start * (1.0 - font.outlineBlur),
                                   inverted_threshold - d_sdf - font.outlineBias / width);
    }

    inner_opacity = pow(inner_opacity, 1.0 / font.gamma);
    return (inner_color * inner_opacity) + (outer_color * (outer_opacity - inner_opacity));
}
