/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "postprocess.inc.slang"

struct Data {
    float gamma;
};

[[vk::binding(2, 0)]] ConstantBuffer<Data> data : register(b2);

float3 linearToSrgb(
    float3 c,
    float gamma = 2.4,
    float threshold = 0.0031308,
    float scaleLow = 12.92,
    float scaleHigh = 1.055,
    float offsetHigh = 0.055) {
    return select(
        c <= threshold,
        c * scaleLow,
        scaleHigh * pow(c, 1.0 / gamma) - offsetHigh
    );
}

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    float4 color = inputImage.Sample(sampler0, input.uv);
    return float4(linearToSrgb(color.rgb, data.gamma), color.a);
}
