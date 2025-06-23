/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "lighting.inc.slang"

float4 getColor(VertexOutput input, Material mat, float4 color) {
    float3 normal;
    if (mat.normalTexture.index != -1) {
        // Get current fragment's normal and transform to world space.
        normal = textures[mat.normalTexture.index].Sample(
                     samplers[mat.normalTexture.samplerIndex],
                     uvTransform(mat.normalTexture, input.uv)).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
        normal = normalize(mul(normal, TBN)) * float3(mat.normalScale, mat.normalScale, 1.0f);
    } else {
        // We don't have a texture, get the calculated normal
        normal = input.normal;
    }

    float3 diffuse = float3(0.0f);
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
            samplers[mat.emissiveTexture.samplerIndex], uvTransform(mat.emissiveTexture, input.uv))).rgb;
    }

    return float4(
        calcLight(input.viewDirection, color.rgb, normal, scene.ambientLight, ispbr, metallic, roughness, emissiveColor),
        color.a);
}