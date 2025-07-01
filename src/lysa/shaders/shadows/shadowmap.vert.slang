/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "shadowmap.inc.slang"

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    Instance instance = instances[instanceIndex];
    float4x4 model = meshInstances[instance.meshInstanceIndex].transform;
    Material mat = materials[instance.materialIndex];
    float4 positionW = mul(model, float4(input.position.xyz, 1.0));
    output.worldPos = positionW;
    output.position = mul(global.lightSpace, positionW);
    output.materialIndex = instance.materialIndex;
    output.uv = float2(input.position.w, input.normal.w);
    return output;
}
