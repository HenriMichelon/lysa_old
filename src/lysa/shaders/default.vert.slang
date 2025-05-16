/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene_data.inc.slang"

ConstantBuffer<Scene>     scene  : register(b0, space0);
StructuredBuffer<Model> models : register(b1, space0);

[[push_constant]]
PushConstants pushConstants : register(b0, space1);

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    float4x4 model = models[pushConstants.modelIndex].transform;

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
    return output;
}
