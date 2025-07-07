/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "postprocess.inc.slang"

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    Texture2D edgeBuffer = textures[DEPTH_BUFFER];
    SamplerState sampler = samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR];

    float2 edge = edgeBuffer.Sample(sampler, input.uv).rg;
    float weight = saturate(max(edge.r, edge.g));
    return float4(weight, weight, weight, 0);
}
