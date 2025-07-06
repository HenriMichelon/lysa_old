/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "postprocess.inc.slang"

struct DataGammaCorrection {
    float gamma;
    float exposure;
};

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

float3 applyExposure(float3 color, float depth, float exposure) {
    if (depth == 1.0) {
        return color;
    } else {
        return float3(1.0) - exp(-color * exposure);
    }}

float4 gammaCorrection(float4 color, float gamma) {
    return float4(linearToSrgb(color.rgb, gamma), color.a);
}

float luminance(float3 v) {
    return dot(v, float3(0.2126f, 0.7152f, 0.0722f));
}

// https://64.github.io/tonemapping/
float3 toneMapReinhard(float3 v) {
    float l = luminance(v);
    float3 tv = v / (1.0f + v);
    return lerp(v / (1.0f + l), tv, tv);
}

//https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 toneMapACES(float3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}
