/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "common.inc.slang"

struct DataBlur {
    int kernelSize;
    float strength;
};

[[vk::binding(1, 0)]] ConstantBuffer<DataBlur> data : register(b1, space0);

float4 fragmentMain(VertexOutput input) : SV_TARGET {
    return gaussianBlur(inputImage, input.uv, data.kernelSize, data.strength, params.imageSize);
}
