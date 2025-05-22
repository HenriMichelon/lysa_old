/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

StructuredBuffer<Material> materials : register(t1);

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    return materials[input.materialIndex].albedoColor;
//     return float4(0., 0., 0., 1.0);
}
