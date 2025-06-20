/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "forward.inc.slang"

struct FragmentOutput {
    float4 accum  : SV_TARGET0;
    float  reveal : SV_TARGET1;
};

float weightDepth(float z, float4 color) {
    return clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - z * 0.9, 3.0), 1e-2, 3e3);
}

float weightColor(float z, float4 color) {
    return max(min(1.0, max(max(color.r, color.g), color.b) * color.a), color.a) *
        clamp(0.03 / (1e-5 + pow(z / 200, 4.0)), 1e-2, 3e3);
}

FragmentOutput fragmentMain(VertexOutput input) {
    float4 color = getColor(input, input.materialIndex);
    float weight = weightDepth(input.position.z, color);

    FragmentOutput output;
    output.accum = float4(color.rgb * color.a, color.a) * weight;
    output.reveal = color.a;
    return output;
}
