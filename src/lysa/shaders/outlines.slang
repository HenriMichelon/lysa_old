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
    float alpha     : TEXCOORD0;
};

struct GlobalUniform {
    float4x4 projection;
    float4x4 view;
}

struct PushConstants {
    float  scale;
    float4 color;
};

[[vk::binding(0, 0)]] ConstantBuffer<GlobalUniform> global  : register(b0, space0);
[[push_constant]]
PushConstants pushConstants : register(b0, space1);

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    float scale = pushConstants.scale;
    output.position = mul(global.projection, mul(global.view,  float4(input.position * scale, 1.0)));
//     output.position *= float4(scale, scale, scale, 1.0f);
    output.color = pushConstants.color.rgb;
    output.alpha = pushConstants.color.a;
    return output;
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    return float4(input.color, input.alpha);
}
