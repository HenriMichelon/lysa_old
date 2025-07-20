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

VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    float2 pos = 2 * (input.position.xy - 0.5); // remap to [-1,1]
    output.position = float4(pos.x, -pos.y, 0, 1);
    output.uv = input.uv;
    output.color = input.color;
    output.uvClip = input.uvClip;
    output.textureIndex = input.textureIndex;
    return output;
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    if (input.textureIndex == -1) {
        return input.color;
    } else {
        if ((input.uv.x > input.uvClip.x) || (input.uv.y < input.uvClip.y)) {
            discard;
        }
        return input.color;
    }
}
