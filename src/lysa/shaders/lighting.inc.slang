/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"


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

float3 calcDirectionalLight(Light light, float3 worldPos, float3 normal, float shininess) {
    float3 L = -light.direction.xyz;
    float3 V = normalize(scene.cameraPosition - worldPos);
    float3 R = reflect(-L, normal);

    float diff = max(dot(normal, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), shininess);

    float3 diffuse = diff * light.color.rgb * light.color.w;
    float3 specular = spec * light.color.rgb * light.color.w;

    return diffuse + specular;
}

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
        normal = normalize(mul(TBN, normal)) * float3(mat.normalScale, mat.normalScale, 1.0f);
    } else {
        // We don't have a texture, get the calculated normal
        normal = input.normal;
    }
//     return float4(normal, 1.0); // debug normals

    float3 lit = 0.0f;
    for (uint i = 0; i < scene.lightsCount; i++) {
        lit += calcDirectionalLight(lights.light[i], input.worldPos, normal, 128.0);
    }

    return float4(color.rgb * (lit + ambient), transparency);
}