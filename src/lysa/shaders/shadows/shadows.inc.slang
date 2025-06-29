/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
static const float SHADOW_FACTOR = 0.1;

float shadowFactor(Light light, int cascadeIndex, float3 worldPos) {
    const float4 shadowCoord = mul(light.lightSpace[cascadeIndex], float4(worldPos, 1.0));
    float3 projCoords = shadowCoord.xyz / shadowCoord.w;
    if (projCoords.z > 1.0) {
       return 1.0;
    }
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords.y = 1.0 - projCoords.y;
    const bool outOfView = (projCoords.x < 0.01 || projCoords.x > 0.99 || projCoords.y < 0.01|| projCoords.y > 0.99);
    if (outOfView) {
       return 1.0;
    }
    const float closestDepth = shadowMaps[light.mapIndex]
        .Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], projCoords.xy).r;
    const float currentDepth = projCoords.z;

    uint width, height;
    shadowMaps[light.mapIndex].GetDimensions(width, height);
    const float2 texelSize = 1.0 / float2(width, height);
    const float bias = 0.0005;
    float shadow = 0.0;
    for(int x = -1; x <= 1; ++x)  {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = shadowMaps[light.mapIndex]
                .Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], projCoords.xy + float2(x, y) * texelSize.xy).r;
            shadow += (currentDepth - bias) > pcfDepth ? SHADOW_FACTOR : 1.0;
        }
    }
    return shadow /= 9.0;
}

struct SampledCube {
    int faceIndex;
    float2 uv;
};

// https://www.gamedev.net/forums/topic/687535-implementing-a-cube-map-lookup-function/5337472/
SampledCube sampleCube(const float3 v) {
	float3 vAbs = abs(v);
	float ma;
	float2 uv;
	int faceIndex;
	if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y) {
		faceIndex = v.z < 0.0 ? 5 : 4;
		ma = 0.5 / vAbs.z;
		uv = float2(v.z < 0.0 ? v.x : -v.x, -v.y);
	}
	else if(vAbs.y >= vAbs.x) {
		faceIndex = v.y < 0.0 ? 3 : 2;
		ma = 0.5 / vAbs.y;
		uv = float2(v.x, v.y < 0.0 ? v.z : -v.z);
	}
	else {
		faceIndex = v.x < 0.0 ? 1 : 0;
		ma = 0.5 / vAbs.x;
		uv = float2(v.x < 0.0 ? -v.z : v.z, -v.y);
	}
	return SampledCube(faceIndex, uv * ma + 0.5);
}

float shadowFactorCubemap(Light light, float3 worldPos) {
    const float3 fragToLight = worldPos - light.position.xyz;
    const float currentDepth = length(fragToLight);
    const float3 dir = normalize(fragToLight);
    const float bias = 0.05;

//     SampledCube sc = sampleCube(dir);
//     float closestDepth = shadowMaps[light.mapIndex + sc.faceIndex]
//         .Sample(samplers[SAMPLER_NEAREST_NEAREST_BORDER_LINEAR], sc.uv).r;
//     closestDepth *= light.farPlane;
//     return (currentDepth - bias) > closestDepth ? SHADOW_FACTOR : 1.0;

    const int samples = 20;
    const float sampleRadius = 0.03; // PCF cone radius
    float shadow = 0.0;
    for (int i = 0; i < samples; ++i) {
        SampledCube sc = sampleCube(dir);
        float sampledDepth = shadowMaps[light.mapIndex + sc.faceIndex]
            .Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR], sc.uv).r;
        sampledDepth *= light.range;
        shadow += (currentDepth - bias) > sampledDepth ? SHADOW_FACTOR : 1.0;
    }
    return shadow /= samples;
}
