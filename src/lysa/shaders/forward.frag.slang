/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

StructuredBuffer<Material> materials : register(b2, space0);

float4 fragmentMain(VertexOutput input) : SV_TARGET {
//     return materials[input.instanceID].albedoColor;
    return float4(0.5, 0.5, 0.5, 1.0);
}
