/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "forward.inc.slang"

float4 fragmentMain(VertexOutput input) : SV_TARGET {
   return getColor(input, input.materialIndex);
}
