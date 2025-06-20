/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"

struct FragmentOutput {
    float4   position : SV_TARGET0; // rgb = world-space position, a=unsued
    float4   normal   : SV_TARGET1; // rgb = world-space normal, a=unused
    float4   albedo   : SV_TARGET2; // rgb = albedo, a unused
//     float    material : SV_TARGET3; // r = material index
};

FragmentOutput fragmentMain(VertexOutput input) : SV_TARGET {
    Material mat = materials[input.meshSurfaceMaterialIndex];

    float4 color = mat.albedoColor;
        if (mat.diffuseTexture.index != -1) {
            color = textures[mat.diffuseTexture.index].Sample(
                samplers[mat.diffuseTexture.samplerIndex],
                uvTransform(mat.diffuseTexture, input.uv));
        }

    float3 normal;
    if (mat.normalTexture.index != -1) {
        // Get current fragment's normal and transform to world space.
        normal = textures[mat.normalTexture.index].Sample(
                     samplers[mat.normalTexture.samplerIndex],
                     uvTransform(mat.normalTexture, input.uv)).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
        normal = normalize(mul(TBN, normal)) * float3(mat.normalScale, mat.normalScale, 1.0f);
    } else {
        // We don't have a texture, get the calculated normal
        normal = input.normal;
    }

    FragmentOutput output;
    output.position = float4(input.worldPos, 0.0);
    output.normal = float4(normal, 0.0);
    output.albedo = float4(color.rgb, 1.0);
//     output.material = asfloat(input.materialIndex);
    return output;
}
