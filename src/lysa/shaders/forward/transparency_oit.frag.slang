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

FragmentOutput fragmentMain(VertexOutput input) {
     Material mat = materials[input.materialIndex];
    float4 color = fetchColor(input.uv, mat);
    if (color.a >= 0.99) {
        discard;
    }
    color = getColor(input, mat, color);

    // https://casual-effects.blogspot.com/2015/03/implemented-weighted-blended-order.html
    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - input.position.z * 0.9, 3.0), 1e-2, 3e3);

    FragmentOutput output;
    output.accum = float4(color.rgb * color.a, color.a) * weight;
    output.reveal = color.a;
    return output;
}
