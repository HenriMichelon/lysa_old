/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "instances.inc.slang"

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    Instance instance = instances[instanceIndex];
    float4x4 model = meshInstances[instance.meshInstanceIndex].transform;
    float4 position = float4(input.position.xyz, 1.0);
    float4 positionW = mul(model, position);
    output.position = mul(scene.projection, mul(scene.view, positionW));
    return output;
}
