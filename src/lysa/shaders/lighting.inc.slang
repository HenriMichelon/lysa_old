/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"
#include "pbr.inc.slang"

static const int LIGHT_DIRECTIONAL = 0;
static const int LIGHT_OMNI        = 1;
static const int LIGHT_SPOT        = 2;

struct Light {
    // light params
    int type; // Light::LightType
    float range;
    float cutOff;
    float outerCutOff;
    float4 position;
    float4 direction;
    float4 color; // RGB + Intensity;
    // shadow map params
    int mapIndex;
    float farPlane;
    uint cascadesCount;
	float _pad0;
    float4 cascadeSplitDepth;
    float4x4 lightSpace[6];
};

struct Lights {
    Light[100] light;
}

[[vk::binding(2, 2)]] ConstantBuffer<Lights> lights : register(b2, space2);

// float3 calcDirectionalLight(Light light, float3 worldPos, float3 normal, float shininess) {
//     float3 L = -light.direction.xyz;
//     float3 V = normalize(scene.cameraPosition - worldPos);
//     float3 R = reflect(-L, normal);
// 
//     float diff = max(dot(normal, L), 0.0);
//     float spec = pow(max(dot(V, R), 0.0), shininess);
// 
//     float3 diffuse = diff * light.color.rgb * light.color.w;
//     float3 specular = spec * light.color.rgb * light.color.w;
// 
//     return diffuse + specular;
// }

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
    const float cosLo,
    const float alphaSq,
    const float alphaDirectLighting
) {
    // https://learnopengl.com/PBR/Theory
    // Half-vector between Li and Lo.
    const float3 Lh = normalize(Li + Lo);

    // Calculate angles between surface normal and various light vectors.
    const float cosLi = max(0.0, dot(N, Li));
    const float cosLh = max(0.0, dot(N, Lh));

    // Calculate Fresnel term for direct lighting.
    const float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
    // Calculate normal distribution for specular BRDF.
    const float D = ndfGGX(cosLh, alphaSq);
    // Calculate geometric attenuation for specular BRDF.
    const float G = gaSchlickGGX(cosLi, cosLo, alphaDirectLighting);

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
    const float3 kD = lerp(float3(1.0) - F, float3(0.0), metallic);

    // Lambert diffuse BRDF.
    // We don't scale by 1/PI for lighting & material units to be more convenient.
    // See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    const float3 diffuseBRDF = kD * albedo;

    // Cook-Torrance specular microfacet BRDF.
    const float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

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
    const float cosLo,
    const float alphaSq,
    const float alphaDirectLighting) {
    return
        light.color.rgb *
        light.color.w *
        Radiance(albedo, normal, viewDirection,  metallic, roughness,  -light.direction.xyz, F0, cosLo, alphaSq, alphaDirectLighting);
}

// float3 calcPointLight(Light light, float3 albedo, float3 normal,float metallic, float roughness, float3 viewDirection, float3 fragPos, float3 F0, const float cosLo, const float alphaSq, const float alphaDirectLighting) {
//     const float attenuation = clamp(1.0 - length(light.position - fragPos)/light.range, 0.0, 1.0);
//     const float3 lightDir = normalize(light.position - fragPos);
//     const float3 diffuse = light.color.rgb * light.color.w * attenuation *
//                         Radiance(albedo, normal, viewDirection, metallic, roughness, lightDir,F0, cosLo, alphaSq, alphaDirectLighting);
//     float intensity = 1.0f;
//     if (light.type == LIGHT_SPOT) {
//         const float theta = dot(lightDir, normalize(-light.direction));
//         const float epsilon = light.cutOff - light.outerCutOff;
//         const float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
//         if (theta <= light.outerCutOff) return float3(0.0f);
//         return min(intensity * diffuse, float3(1.0f));
//     } else {
//         return min(diffuse, float3(1.0f));
//
//     }
// }

float3 calcLight(
    VertexOutput input,
    float3 color,
    float3 normal,
    bool ispbr,
    float metallic,
    float roughness,
    float3 emissiveColor) {
    float3 ambient = scene.ambientLight.rgb * scene.ambientLight.w;
    float3 diffuse = float3(0.0f);
    if (ispbr) {
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
    return float3(diffuse + ambient + emissiveColor);
}