/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"

VertexOutput vertexMain(uint vertexId: SV_VertexID ) {
    Index index = indices[vertexId];
    MeshSurface surface = surfaces[index.surfaceIndex];
    return vertexMVP(
        surface,
        vertices[index.index + surface.firstVertex]);
}