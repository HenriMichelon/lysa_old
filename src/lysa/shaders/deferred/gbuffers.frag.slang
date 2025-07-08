/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "../scene.inc.slang"

struct FragmentOutput {
    float4 position : SV_TARGET0; // rgb = world-space position, a=view position z
    float4 normal   : SV_TARGET1; // rgb = world-space normal, a=roughness
    float4 albedo   : SV_TARGET2; // rgb = albedo,  a=metallic
    float4 emissive : SV_TARGET3; // rgb = emissive color,
};

FragmentOutput fragmentMain(VertexOutput input) : SV_TARGET {
    Material mat = materials[input.materialIndex];
    float4 color = fetchColor(input.uv, mat);
    if (mat.transparency != Transparency.DISABLED && color.a < 0.99) {
       discard;
    }
    float3 normal = fetchNormal(input.uv, input.TBN, input.normal, mat);
    float metallic = fetchMetallic(input.uv, mat);
    float roughness = fetchRoughness(input.uv, mat);
    float3 emissiveColor = fetchEmissiveColor(input.uv, mat);

    FragmentOutput output;
    output.position = float4(input.worldPos, input.viewPos.z);
    output.normal = float4(normal, roughness);
    output.albedo = float4(color.rgb, metallic);
    output.emissive = float4(emissiveColor, 0.0);
    return output;
}
