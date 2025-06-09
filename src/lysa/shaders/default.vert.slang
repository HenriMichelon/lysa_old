/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"

VertexOutput vertexMain(VertexInput input, uint instanceId : SV_InstanceID ) {
    VertexOutput output;

    Instance instance = instances[instanceId];
    MeshSurface surface = surfaces[instance.surfaceIndex];
    float4x4 model = models[instance.modelIndex].transform;

    float4 position = float4(input.position.xyz, 1.0);
    float4 positionW = mul(model, position);
    output.worldPos = positionW.xyz;

    output.position = mul(scene.projection, mul(scene.view, positionW));

    float3 normalW = mul((float3x3)model, input.normal.xyz);
    output.normal = normalize(normalW);

    float3 tangentW = mul((float3x3)model, input.tangent.xyz * input.tangent.w);
    output.tangent = normalize(tangentW);
    output.bitangent = normalize(cross(normalW, tangentW));

    output.uv = float2(input.position.w, input.normal.w);
    output.materialIndex = surface.materialIndex;
    return output;
}