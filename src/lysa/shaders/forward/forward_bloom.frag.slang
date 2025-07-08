/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "forward.inc.slang"

struct FragmentOutput {
    float4 color : SV_TARGET0;
    float4 brightness : SV_TARGET1;
};

FragmentOutput fragmentMain(VertexOutput input)  {
    Material mat = materials[input.materialIndex];
    float4 color = fetchColor(input.uv, mat);
    if (mat.transparency != Transparency.DISABLED && color.a < 0.99) {
       discard;
    }
    FragmentOutput output;
    output.color = getColor(input, mat, color);
    float brightness = dot(output.color.rgb, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        output.brightness = float4(output.color.rgb, 1.0);
    else
        output.brightness = float4(0.0, 0.0, 0.0, 1.0);
    return output;
}
