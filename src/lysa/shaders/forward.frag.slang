/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    Material mat = materials[input.materialIndex];
    float4 color = mat.albedoColor;
    if (mat.diffuseTexture.index != -1) {
        color *= textures[mat.diffuseTexture.index].Sample(
            samplers[mat.diffuseTexture.samplerIndex],
            uvTransform(mat.diffuseTexture, input.uv));
    }

    if (((mat.transparency == TRANSPARENCY_SCISSOR) ||
        (mat.transparency == TRANSPARENCY_SCISSOR_ALPHA))
        && (color.a < mat.alphaScissor)) {
        discard;
    }
    const float transparency = (
        mat.transparency == TRANSPARENCY_ALPHA ||
        mat.transparency == TRANSPARENCY_SCISSOR_ALPHA) ? color.a : 1.0;
    float3 ambient = scene.ambientLight.rgb * scene.ambientLight.w;

    float3 normal;
    if (mat.normalTexture.index != -1) {
        // Get current fragment's normal and transform to world space.
        normal = normalize(
            2.0 * textures[mat.normalTexture.index].Sample(
                samplers[mat.normalTexture.samplerIndex],
                uvTransform(mat.normalTexture, input.uv)).rgb - 1.0);
        // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_normaltextureinfo_scale
        float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
        normal = normalize(mul(TBN, normal)) * float3(mat.normalScale, mat.normalScale, 1.0f);
    } else {
        // We don't have a texture, get the calculated normal
        normal = input.normal;
    }
//     return float4(normal, 1.0); // debug normals

    return float4(color.rgb * ambient, transparency);
}
