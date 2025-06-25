/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "shadowmap.inc.slang"

struct VertexInput {
    float4 position : POSITION; // position + uv.x
#ifdef __SPIRV__
    uint instanceId : SV_StartInstanceLocation;
    #define instanceIndex input.instanceId
#endif
}

#ifndef __SPIRV__
cbuffer IndirectRootConstant : register(b0, space4) {
    uint instanceIndex;
};
#endif

struct VertexOutput {
    float4 position : SV_POSITION;
}

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    Instance instance = instances[instanceIndex];
    float4x4 model = meshInstances[instance.meshInstanceIndex].transform;
    Material mat = materials[instance.materialIndex;];
    if (pushConstants.transparency != 0) {
        // discard in case of transparency, any mode
        gl_Position = vec4(0);
    } else {
        float4 positionW = mul(model, float4(input.position.xyz, 1.0));
        gl_Position = global.lightSpace[pushConstants.lightSpaceIndex] * ;
    }
}