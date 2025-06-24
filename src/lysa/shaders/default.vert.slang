/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "instances.inc.slang"

float3x3 inverse3x3(float3x3 m) {
    float3 a = m[0];
    float3 b = m[1];
    float3 c = m[2];

    float3 r0 = cross(b, c);
    float3 r1 = cross(c, a);
    float3 r2 = cross(a, b);

    float invDet = 1.0 / dot(r2, c);

    return float3x3(r0, r1, r2) * invDet;
}


VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;

    Instance instance = instances[instanceIndex];
    MeshSurface surface = meshSurfaces[instance.meshSurfaceIndex];
    float4x4 model = meshInstances[instance.meshInstanceIndex].transform;

    float4 position = float4(input.position.xyz, 1.0);
    float4 positionW = mul(model, position);
    output.worldPos = positionW.xyz;

    output.position = mul(scene.projection, mul(scene.view, positionW));

    float3x3 normalMatrix = transpose(inverse3x3((float3x3)model));
    float3 normalW = mul(normalMatrix, input.normal.xyz);
    output.normal = normalize(normalW);

    float3 tangentW = mul((float3x3)model, input.tangent.xyz * input.tangent.w);
    output.tangent = normalize(tangentW);
    output.bitangent = normalize(cross(normalW, tangentW));

    output.uv = float2(input.position.w, input.normal.w);
    output.materialIndex = instance.materialIndex;
    output.meshSurfaceMaterialIndex = instance.meshSurfaceMaterialIndex;
    output.viewDirection = normalize(scene.cameraPosition - output.worldPos);
    return output;
}

