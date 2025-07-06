/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../lighting/lighting.inc.slang"

float4 getColor(VertexOutput input, Material mat, float4 color) {
    float3 normal = fetchNormal(input, mat);
    float metallic = fetchMetallic(input, mat);
    float roughness = fetchRoughness(input, mat);
    float3 emissiveColor = fetchEmissiveColor(input, mat);
//     return float4(normal, 1.0);
    //return float4(input.viewPos.z, 0.0, 0.0, 1.0);
    return float4(
        calcLight(
            input.viewDirection,
            input.worldPos,
            color.rgb,
            normal,
            scene.ambientLight,
            1.0,
            metallic,
            roughness,
            emissiveColor,
            input.viewPos.z),
        color.a);
}