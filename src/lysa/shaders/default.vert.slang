/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

VertexOutput vertexMain(uint vertexId: SV_VertexID ) {
    Index index = indices[vertexId];
    InstanceData instanceData = instancesData[index.surfaceIndex];
    return vertexMVP(
        instanceData,
        vertices[index.index + instanceData.firstVertex]);
}