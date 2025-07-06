/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "common.inc.slang"

[[vk::binding(1, 0)]] ConstantBuffer<DataGammaCorrection> data : register(b1, space0);

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    float4 color = textures[INPUT_BUFFER].Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    float depth = textures[DEPTH_BUFFER].Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).r;
    if (params.applyBloom == 1) {
        color.rgb += textures[BLOOM_BUFFER].Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv).rgb;
    }
    color.rgb = applyExposure(color.rgb, depth, data.exposure);
    color.rgb = toneMapReinhard(color.rgb);
    return gammaCorrection(color, data.gamma);
}
