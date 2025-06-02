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
    if (material.diffuseTexture.index != -1) {
//         color *= textures[material.diffuseTexture.index].Sample(sampler2, uvTransform(material.diffuseTexture, input.uv));
        color *= textures[material.diffuseTexture.index].Sample(sampler2, input.uv);
    }
    if (((material.transparency == TRANSPARENCY_SCISSOR) || (material.transparency == TRANSPARENCY_SCISSOR_ALPHA)) && (color.a < material.alphaScissor)) {
        discard;
    }
    const float transparency = (material.transparency == TRANSPARENCY_ALPHA || material.transparency == TRANSPARENCY_SCISSOR_ALPHA) ? color.a : 1.0f;
    float3 ambient = scene.ambientLight.rgb * scene.ambientLight.w;

    return float4(color.rgb * ambient, transparency);
//     return float4(input.uv.x, input.uv.y, material.parameters[0].r, transparency);
//     return float4(1.0, 0.0, material.parameters[0].r, 1.0) * scene.ambientLight;
}
