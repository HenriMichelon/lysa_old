/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "common.inc.slang"

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    float4 color = textures[INPUT_BUFFER].Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv);
    const float grey = dot(color.rgb, float3(0.299, 0.587, 0.114));
    return float4(grey * float3(1.0, 1.0, 1.0), color.a);
}
