/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "shadowmap.inc.slang"

float fragmentMain(VertexOutput input) : SV_Depth {
    Material mat = materials[input.materialIndex];
    float4 color = fetchColor(input, mat);
    if (mat.transparency != Transparency.DISABLED && color.a < 0.1) {
       discard;
    }
    return input.position.z;
}