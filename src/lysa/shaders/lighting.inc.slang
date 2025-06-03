/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

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
