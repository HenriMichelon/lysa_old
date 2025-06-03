/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

VertexOutput vertexMain(uint vertexId: SV_VertexID) {
    VertexOutput output;
    Index index = indices[vertexId];
    InstanceData instanceData = instancesData[index.surfaceIndex];
    Vertex input = vertices[index.index + instanceData.firstVertex];
    float4x4 model = instanceData.transform;
    float4 position = float4(input.position.xyz, 1.0);
    float4 positionW = mul(model, position);
    output.position = mul(scene.projection, mul(scene.view, positionW));
    return output;
}