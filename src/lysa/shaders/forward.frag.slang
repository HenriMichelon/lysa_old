/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    Material material = materials[input.materialIndex];
    float4 color = material.albedoColor;
    if (material.diffuseTextureIndex != -1) {
        color *= textures[material.diffuseTextureIndex].Sample(sampler1, input.uv);
    }
    return color * scene.ambientLight;
//     return float4(input.uv.x, input.uv.y, material.parameters[0].r, 1.0) * scene.ambientLight;
//     return float4(1.0, 0.0, material.parameters[0].r, 1.0) * scene.ambientLight;
}
