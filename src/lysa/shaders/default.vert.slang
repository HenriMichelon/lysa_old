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
    MeshSurface surface = meshSurfaces[instance.meshSurfaceIndex];

    float4x4 model = meshInstances[instance.meshInstanceIndex].transform;
    float4 positionW = mul(model, float4(input.position.xyz, 1.0));

    float3 normalW = normalize(mul(float3x3(model), input.normal.xyz));
    float3 tangentW = normalize(mul(float3x3(model), input.tangent.xyz));
    float3 bitangentW = normalize(cross(normalW, tangentW) * input.tangent.w);

    float4 viewPos = mul(scene.view, positionW);

    output.worldPos = positionW.xyz;
    output.position = mul(scene.projection, viewPos);
    output.normal = normalW;
    output.uv = float2(input.position.w, input.normal.w);
    output.materialIndex = instance.materialIndex;
    output.meshSurfaceMaterialIndex = instance.meshSurfaceMaterialIndex;
    output.viewDirection = normalize(scene.cameraPosition - output.worldPos);
    output.viewPos = viewPos;
    output.TBN = float3x3(tangentW, bitangentW, normalW);
    return output;
}

