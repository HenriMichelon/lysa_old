/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "shadowmap.inc.slang"

FragmentOutput fragmentMain(VertexOutput input) {
    Material mat = materials[input.materialIndex];
    float4 color = fetchColor(input, mat);
    FragmentOutput output;
    if (mat.transparency != Transparency.DISABLED && color.a < global.transparencyColorScissor) {
        if (color.a < global.transparencyScissor) {
            discard;
        }
        output.transparencyColor = color;
    } else {
        output.transparencyColor = float4(0.0, 0.0, 0.0, 1.0);
    }
    output.depth = length(input.worldPos.xyz - global.lightPosition.xyz) / global.lightPosition.w;
    return output;
}