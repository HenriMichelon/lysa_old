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
    int textureIndex: TEXTURE;
    int fontIndex   : FONT;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
    nointerpolation int textureIndex : TEXTURE;
    nointerpolation int fontIndex    : FONT;
};

struct GlobalUniform {
    float4x4 projection;
    float4x4 view;
}
