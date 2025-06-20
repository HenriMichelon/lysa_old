/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "lighting.inc.slang"

float4 getColor(VertexOutput input, uint materialIndex) {
    Material mat = materials[materialIndex];
    float4 color = mat.albedoColor;
    if (mat.diffuseTexture.index != -1) {
        color = textures[mat.diffuseTexture.index].Sample(
            samplers[mat.diffuseTexture.samplerIndex],
            uvTransform(mat.diffuseTexture, input.uv));
    }

    if (((mat.transparency == Transparency::SCISSOR) ||
        (mat.transparency == Transparency::SCISSOR_ALPHA))
        && (color.a < mat.alphaScissor)) {
        discard;
    }
    const float transparency = (
        mat.transparency == Transparency::ALPHA ||
        mat.transparency == Transparency::SCISSOR_ALPHA) ? color.a : 1.0;
    float3 ambient = scene.ambientLight.rgb * scene.ambientLight.w;

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
    if (mat.metallicFactor != -1) {
        float metallic  = mat.metallicTexture.index == -1 ?
            mat.metallicFactor :
            mat.metallicFactor *
                textures[mat.metallicTexture.index].Sample(samplers[mat.metallicTexture.samplerIndex],
                uvTransform(mat.metallicTexture, input.uv)).b;
        float roughness = mat.roughnessTexture.index == -1 ?
            mat.roughnessFactor :
            mat.roughnessFactor *
                textures[mat.roughnessTexture.index].Sample(samplers[mat.roughnessTexture.samplerIndex],
                uvTransform(mat.roughnessTexture, input.uv)).g;
        // for ndfGGX()
        const float alphaSq = roughness * roughness * roughness * roughness;
        // Radiance()
        const float r = (roughness + 1.0);
        const float alphaDirectLighting = (r * r)  * 0.125;
        // Fresnel reflectance at normal incidence (for metals use albedo color).
        const float3 F0 = lerp(Fdielectric, color.rgb, metallic);
        // Specular reflection vector.
        const float cosLo = max(0.0, dot(normal, input.viewDirection));
        // Calculate the diffuse light from the scene's lights
        for (uint i = 0; i < scene.lightsCount; i++) {
            Light light = lights.light[i];
            float factor = 1.0f;
            switch (light.type) {
                case LIGHT_DIRECTIONAL: {
                    diffuse += factor * calcDirectionalLight(
                        light, color.rgb, normal, metallic, roughness,
                        input.viewDirection, F0, cosLo, alphaSq, alphaDirectLighting);
                    break;
                }
            }
            diffuse = clamp(diffuse, 0.0, 1.0);
        }
    }
    float3 emissiveColor = mat.emissiveFactor;
    if (mat.emissiveTexture.index != -1) {
        emissiveColor *= toLinear(textures[mat.emissiveTexture.index].Sample(
            samplers[mat.emissiveTexture.samplerIndex], uvTransform(mat.emissiveTexture, input.uv))).rgb;
    }
    diffuse += emissiveColor * mat.emissiveStrength;
 
    return float4(ambient + diffuse, transparency);
}