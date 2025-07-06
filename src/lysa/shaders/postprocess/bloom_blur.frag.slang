/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "common.inc.slang"

struct DataBlur {
    uint   kernelSize;
    float4 weights[9*9];
    float2 texelSize;
};

[[vk::binding(1, 0)]] ConstantBuffer<DataBlur> data : register(b1, space0);

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    int halfKernel = data.kernelSize / 2;
    float4 color = float4(0.0);
    for (int i = -halfKernel; i <= halfKernel; i++) {
        for (int j = -halfKernel; j <= halfKernel; j++) {
            float2 offset = float2(float(i), float(j)) * data.texelSize;
            int index = (i + halfKernel) * data.kernelSize + (j + halfKernel);
            color += textures[INPUT_BUFFER].Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], input.uv + offset) * data.weights[index].x;
        }
    }
    return color;
}
