/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

[[vk::binding(0, 3)]] StructuredBuffer<Index> indices : register(t0, space3);

VertexOutput vertexMain(uint vertexId: SV_VertexID ) {
    VertexOutput output;

    Index index = indices[vertexId];
    InstanceData instanceData = instancesData[index.surfaceIndex];
    Vertex input = vertices[index.index + instanceData.firstVertex];
    float4x4 model = instanceData.transform;

    float4 localPos = float4(input.position, 1.0);
    float4 worldPos = mul(model, localPos);
    output.worldPos = worldPos.xyz;

    output.position = mul(scene.projection, mul(scene.view, worldPos));

    float3 normalW = mul((float3x3)model, input.normal);
    output.normal = normalize(normalW);

    float3 tangentW = mul((float3x3)model, input.tangent);
    output.tangent = normalize(tangentW);
    output.bitangent = normalize(cross(normalW, tangentW));

    output.uv = input.uv;
    output.materialIndex = instanceData.materialIndex;
    return output;
}
