/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"

struct FragmentOutput {
    float4   position : SV_TARGET0; // rgb = world-space position, a=metallic
    float4   normal   : SV_TARGET1; // rgb = world-space normal, a=roughness
    float4   albedo   : SV_TARGET2; // rgb = albedo, a unused
    float4   emissive : SV_TARGET3; // rgb = emissive color, a = have material+roughness?
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

    float metallic;
    float roughness;
    bool ispbr =  (mat.metallicFactor != -1);
    if (ispbr) {
         metallic  = mat.metallicTexture.index == -1 ?
             mat.metallicFactor :
             mat.metallicFactor *
                 textures[mat.metallicTexture.index].Sample(samplers[mat.metallicTexture.samplerIndex],
                 uvTransform(mat.metallicTexture, input.uv)).b;
         roughness = mat.roughnessTexture.index == -1 ?
             mat.roughnessFactor :
             mat.roughnessFactor *
                 textures[mat.roughnessTexture.index].Sample(samplers[mat.roughnessTexture.samplerIndex],
                 uvTransform(mat.roughnessTexture, input.uv)).g;
    }
    float3 emissiveColor = mat.emissiveFactor;
    if (mat.emissiveTexture.index != -1) {
         emissiveColor *= toLinear(textures[mat.emissiveTexture.index].Sample(
             samplers[mat.emissiveTexture.samplerIndex], uvTransform(mat.emissiveTexture, input.uv))).rgb
             * mat.emissiveStrength;
    }

    FragmentOutput output;
    output.position = float4(input.worldPos, metallic);
    output.normal = float4(normal, roughness);
    output.albedo = float4(color.rgb, 1.0);
    output.emissive = float4(emissiveColor, ispbr ? 1.0 : 0.0);
    return output;
}
