/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

struct VertexInput {
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

struct GlobalUniform {
    float4x4 projection;
    float4x4 view;
}

[[vk::binding(0, 0)]] ConstantBuffer<GlobalUniform> global  : register(b0, space0);

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    output.position = mul(global.projection, mul(global.view,  float4(input.position, 1.0)));
    output.color = input.color;
    return output;
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    return float4(input.color, 1.0);
}
