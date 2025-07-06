/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../scene.inc.slang"
#include "pbr.inc.slang"
#include "../shadows/shadows.inc.slang"

float3 Radiance(
    // base color
    const float3 albedo,
    // Fragment normal in world space
    const float3 normal,
    // Angle between surface normal and outgoing light direction.
    const float3 viewDirection,
    // Material properties
    const float metallic, float roughness,
    // Direction to light
    const float3 dirToLight,
    // Fresnel reflectance at normal incidence
    const float3 F0,
    // Specular reflection vector.
    const float cosLo) {
    // Half-vector between Li and Lo.
    float3 Lh = normalize(dirToLight + viewDirection);

    // Calculate angles between surface normal and various light vectors.
    float cosLi = max(0.0, dot(normal, dirToLight));
    float cosLh = max(0.0, dot(normal, Lh));

    // Calculate Fresnel term for direct lighting.
    float3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, viewDirection)));
    // Calculate normal distribution for specular BRDF.
    float D = ndfGGX(cosLh, roughness);
    // Calculate geometric attenuation for specular BRDF.
    float G = gaSchlickGGX(cosLi, cosLo, roughness);

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);

    // Lambert diffuse BRDF.
    // We don't scale by 1/PI for lighting & material units to be more convenient.
    // See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    float3 diffuseBRDF = kd * albedo;

    // Cook-Torrance specular microfacet BRDF.
    float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

    // Total contribution for this light.
//     return albedo * cosLi;
    return (diffuseBRDF + specularBRDF) * cosLi;
}

float3 calcDirectionalLight(
    const Light light,
    const float3 albedo,
    const float3 normal,
    const float metallic,
    const float roughness,
    const float3 viewDirection,
    const float3 F0,
    const float cosLo) {
    return light.color.rgb * light.color.w *
        Radiance(albedo, normal, viewDirection,  metallic, roughness,  -light.direction.xyz, F0, cosLo);
}

float3 calcPointLight(
    const Light light,
    const float3 albedo,
    const float3 normal,
    float metallic,
    float roughness,
    const float3 viewDirection,
    const float3 worldPos,
    const float3 F0,
    const float cosLo) {
    const float attenuation = clamp(1.0 - length(light.position.xyz - worldPos)/light.range, 0.0, 1.0);
    const float3 lightDir = normalize(light.position.xyz - worldPos);
    const float3 diffuse = light.color.rgb * light.color.w * attenuation
              * Radiance(albedo, normal, viewDirection, metallic, roughness, lightDir, F0, cosLo);
    if (light.type == LIGHT_SPOT) {
        const float theta = dot(lightDir, normalize(-light.direction.xyz));
        const float epsilon = light.cutOff - light.outerCutOff;
        const float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        if (theta <= light.outerCutOff) {
            return float3(0.0f);
        }
        return intensity * diffuse;
    } else {
        return diffuse;
    }
}

float3 calcLight(
    float3 viewDirection,
    float3 worldPos,
    float3 color,
    float3 normal,
    float4 ambientColor,
    float ambientOcclusion,
    float metallic,
    float roughness,
    float3 emissiveColor,
    float viewPosZ) {
    float3 ambient = ambientColor.rgb * ambientColor.w * color * ambientOcclusion;
    float3 diffuse = float3(0.0f);
    // Angle between surface normal and outgoing light direction.
    float cosLo = max(0.0, dot(normal, viewDirection));
    // Fresnel reflectance at normal incidence (for metals use albedo color).
    float3 F0 = lerp(Fdielectric, color.rgb, metallic);
    // Calculate the diffuse light from the scene's lights
    for (uint lightIndex = 0; lightIndex < scene.lightsCount; lightIndex++) {
        Light light = lights.light[lightIndex];
        float3 factor = float3(1.0, 1.0, 1.0);
        switch (light.type) {
            case LIGHT_DIRECTIONAL: {
                if (light.mapIndex != -1) {
                    // We have a cascaded shadow map,
                    // get cascade index maps for the current fragment's view Z position
                    int cascadeIndex = 0;
                    for (int index = 0; index < light.cascadesCount; index++) {
                        if (-viewPosZ > light.cascadeSplitDepth[index]) {
                            cascadeIndex = index + 1;
                        }
                    }
                    factor = shadowFactor(light, cascadeIndex, worldPos);
                    // Transitioning between cascades
                    if (factor.x >= 0.2 && (cascadeIndex > 0) && cascadeIndex < light.cascadesCount) {
                        float maxDepth = light.cascadeSplitDepth[cascadeIndex];
                        float fade = smoothstep(maxDepth - 0.5, maxDepth, -viewPosZ);
                        factor = lerp(shadowFactor(light, cascadeIndex-1, worldPos), factor, fade);
                    }
                     if ((factor.x >= 0.2) && (cascadeIndex < light.cascadesCount)) {
                        float3 nextFactor = shadowFactor(light, cascadeIndex +1, worldPos);
                        if (nextFactor.x < 0.2f) factor = nextFactor;
                    }
                    // Get the shadow factor for the cascade
                    // Display the cascade splits for debug
                    /*switch (cascadeIndex) {
                        case 0 :
                            color.rgb *= float3(1.0f, 0.25f, 0.25f);
                            break;
                        case 1 :
                            color.rgb *= float3(0.25f, 1.0f, 0.25f);
                            break;
                        case 2 :
                            color.rgb *= float3(0.25f, 0.25f, 1.0f);
                            break;
                        case 3 :
                            color.rgb *= float3(1.0f, 1.0f, 0.25f);
                            break;
                    }*/
                }
                diffuse += factor * calcDirectionalLight(
                    light, color.rgb, normal, metallic, roughness,
                    viewDirection, F0, cosLo);
                break;
            }
            case LIGHT_OMNI:
                if (distance(worldPos, light.position.xyz) <= light.range) {
                    if (light.mapIndex != -1) {
                       factor = shadowFactorCubemap(light, worldPos);
                    }
                    diffuse += factor * calcPointLight(
                        light, color.rgb, normal, metallic, roughness,
                        viewDirection, worldPos, F0, cosLo);
                }
                break;
            case LIGHT_SPOT: {
                if (distance(worldPos, light.position.xyz) <= light.range) {
                    if (light.mapIndex != -1) {
                        factor = shadowFactor(light, 0, worldPos);
                    }
                    diffuse += factor * calcPointLight(
                        light, color.rgb, normal, metallic, roughness,
                        viewDirection, worldPos, F0, cosLo);
                }
                break;
            }
        }
    }
    return float3(diffuse + ambient + emissiveColor);
}
