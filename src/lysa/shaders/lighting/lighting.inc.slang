/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../scene.inc.slang"
#include "pbr.inc.slang"

static const float SHADOW_FACTOR = 0.2;

float shadowFactor(Light light, int cascadeIndex, float3 worldPos, float2 uv) {
    const float4 shadowCoord = mul(light.lightSpace[cascadeIndex], float4(worldPos, 1.0));
    float3 projCoords = shadowCoord.xyz / shadowCoord.w;
    if (projCoords.z > 1.0) {
       return 1.0;
    }
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.001f || projCoords.x > 0.999f || projCoords.y < 0.001f || projCoords.y > 0.999f);
    if (outOfView) {
       return 1.0;
    }
    const float closestDepth = shadowMaps[light.mapIndex]
        .Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], uv).r;
    const float currentDepth = projCoords.z;
    return currentDepth > closestDepth  ? 0.0 : 1.0;
//    float bias = max(0.05 * (1.0 - dot(normal, light.direction)), 0.005);
//    float bias = 0.005;
//    return shadowCoord.w; //currentDepth - bias > closestDepth ? SHADOW_FACTOR : 1.0;

    // PCF
//     uint width, height;
//     shadowMaps[light.mapIndex].GetDimensions(width, height);
//     const float2 texelSize = 1.0 / float2(width, height);
//     const float bias   = 0.0005;
//     float shadow       = 0.0;
//     for(int x = -1; x <= 1; ++x)  {
//         for(int y = -1; y <= 1; ++y) {
//             float pcfDepth = shadowMaps[light.mapIndex]
//                 .Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], projCoords.xy + float2(x, y) * texelSize.xy).r;
//             shadow += (currentDepth - bias) > pcfDepth ? SHADOW_FACTOR : 1.0;
//         }
//     }
//     return shadow /= 9.0;
}


float3 Radiance(
    // base color
    const float3 albedo,
    // Fragment normal in world space
    const float3 N,
    // Angle between surface normal and outgoing light direction.
    const float3 Lo,
    // Material properties
    const float metallic, float roughness,
    // Direction to light
    const float3 Li,
    // Fresnel reflectance at normal incidence
    const float3 F0,
    // Specular reflection vector.
    const float cosLo) {
   		// Half-vector between Li and Lo.
   		float3 Lh = normalize(Li + Lo);

   		// Calculate angles between surface normal and various light vectors.
   		float cosLi = max(0.0, dot(N, Li));
   		float cosLh = max(0.0, dot(N, Lh));

   		// Calculate Fresnel term for direct lighting.
   		float3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
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
   		return (diffuseBRDF + specularBRDF) * cosLi;
}

float3 calcDirectionalLight(
    Light light,
    float3 albedo,
    float3 normal,
    float metallic,
    float roughness,
    float3 viewDirection,
    float3 F0,
    const float cosLo) {
    return
        light.color.rgb *
        light.color.w *
        Radiance(albedo, normal, viewDirection,  metallic, roughness,  -light.direction.xyz, F0, cosLo);
}

float3 calcPointLight(
    Light light,
    float3 albedo,
    float3 normal,
    float metallic,
    float roughness,
    float3 viewDirection,
    float3 worldPos,
    float3 F0,
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
    float2 uv,
    float3 viewDirection,
    float3 worldPos,
    float3 color,
    float3 normal,
    float4 ambientColor,
    float metallic,
    float roughness,
    float3 emissiveColor) {
    float3 ambient = ambientColor.rgb * ambientColor.w * color;
    float3 diffuse = float3(0.0f);
    // Angle between surface normal and outgoing light direction.
    float cosLo = max(0.0, dot(normal, viewDirection));
    // Fresnel reflectance at normal incidence (for metals use albedo color).
    float3 F0 = lerp(Fdielectric, color.rgb, metallic);
    // Calculate the diffuse light from the scene's lights
    for (uint i = 0; i < scene.lightsCount; i++) {
        Light light = lights.light[i];
        float factor = 1.0f;
        switch (light.type) {
            case LIGHT_DIRECTIONAL: {
                diffuse += factor * calcDirectionalLight(
                    light, color.rgb, normal, metallic, roughness,
                    viewDirection, F0, cosLo);
                break;
            }
            case LIGHT_OMNI:
                if (distance(worldPos, light.position.xyz) <= light.range) {
                    diffuse += factor * calcPointLight(
                        light, color.rgb, normal, metallic, roughness,
                        viewDirection, worldPos, F0, cosLo);
                }
                break;
            case LIGHT_SPOT: {
                if (distance(worldPos, light.position.xyz) <= light.range) {
//                     if (light.mapIndex != -1) {
//                         factor = shadowFactor(light, 0, worldPos, uv);
//                         return float3(factor, factor, factor);
//                     }
                    diffuse += factor * calcPointLight(
                        light, color.rgb, normal, metallic, roughness,
                        viewDirection, worldPos, F0, cosLo);
                }
                break;
            }
        }
        diffuse = clamp(diffuse, 0.0, 1.0);
    }
    return float3(diffuse + ambient + emissiveColor);
}
