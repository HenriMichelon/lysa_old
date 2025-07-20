/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

struct VertexInput {
    float3 position : POSITION;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
    float2 uvClip   : CLIP;
    int textureIndex: TEXTURE;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
    nointerpolation float2 uvClip    : CLIP;
    nointerpolation int textureIndex : TEXTURE;
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
    return float4(input.color.rgb, 1.0);
}
