/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "lighting.inc.slang"

float4 getColor(VertexOutput input, Material mat, float4 color) {
    float3 normal = fetchNormal(input, mat);
    float metallic;
    float roughness;
    bool ispbr =  (mat.metallicFactor != -1);
    if (ispbr) {
        metallic  = fetchMetallic(input, mat);
        roughness = fetchRoughness(input, mat);
    }
    float3 emissiveColor = fetchEmissiveColor(input, mat);
    return float4(
        calcLight(input.viewDirection, input.worldPos, color.rgb, normal, scene.ambientLight, ispbr, metallic, roughness, emissiveColor),
        color.a);
}