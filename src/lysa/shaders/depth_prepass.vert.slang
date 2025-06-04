/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"

VertexOutput vertexMain(uint vertexId: SV_VertexID) {
    VertexOutput output;
    Index index = indices[vertexId];
    MeshSurface surface = surfaces[index.surfaceIndex];
    Vertex input = vertices[index.index + surface.firstVertex];
    float4x4 model = models[surface.modelIndex].transform;
    float4 position = float4(input.position.xyz, 1.0);
    float4 positionW = mul(model, position);
    output.position = mul(scene.projection, mul(scene.view, positionW));
    return output;
}