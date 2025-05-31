/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    Material material = materials[input.materialIndex];
    return material.albedoColor * scene.ambientLight;
//     return float4(input.uv.x, input.uv.y, material.parameters[0].r, 1.0) * scene.ambientLight;
}
