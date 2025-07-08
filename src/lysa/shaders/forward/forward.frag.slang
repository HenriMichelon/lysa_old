/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "forward.inc.slang"

float4 fragmentMain(VertexOutput input) : SV_Target {
    Material mat = materials[input.materialIndex];
    float4 color = fetchColor(input.uv, mat);
    if (mat.transparency != Transparency.DISABLED && color.a < 0.99) {
       discard;
    }
    return getColor(input, mat, color);
}
