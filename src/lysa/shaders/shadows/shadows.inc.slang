/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
static const float SHADOW_FACTOR = 0.0;

[[vk::binding(3, 2)]] Texture2D shadowMaps[] : register(t3, space2);
[[vk::binding(0, 4)]] Texture2D shadowTransparencyColorMaps[] : register(t0, space4);

float3 shadowFactor(Light light, int cascadeIndex, float3 worldPos) {
    const float4 shadowCoord = mul(light.lightSpace[cascadeIndex], float4(worldPos, 1.0));
    float3 projCoords = shadowCoord.xyz / shadowCoord.w;
    if (projCoords.z > 1.0) {
       return 1.0;
    }
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.01 || projCoords.x > 0.99 || projCoords.y < 0.01|| projCoords.y > 0.99);
    if (outOfView) {
       return 1.0;
    }
    projCoords.y = 1.0 - projCoords.y;
    const float closestDepth = shadowMaps[light.mapIndex + cascadeIndex]
        .Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR_NOANI], projCoords.xy).r;
    const float currentDepth = projCoords.z;

    uint width, height;
    shadowMaps[light.mapIndex + cascadeIndex].GetDimensions(width, height);
    const float2 texelSize = 1.0 / float2(width, height);
    const float bias = 0.005;
    float shadow = 0.0;

    [unroll]
    for(int x = -1; x <= 1; ++x)  {
        [unroll]
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = shadowMaps[light.mapIndex + cascadeIndex]
                .Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR_NOANI], projCoords.xy + float2(x, y) * texelSize.xy).r;
            shadow += (currentDepth - bias) > pcfDepth ? SHADOW_FACTOR : 1.0;
        }
    }
    float3 factor = float3(shadow /= 9.0);
    const float4 transparencyColor = shadowTransparencyColorMaps[light.mapIndex + cascadeIndex]
        .Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR_NOANI], projCoords.xy);
    if (transparencyColor.a != 1.0) {
        return lerp(transparencyColor.rgb, float3(1.0), factor);
    }
    return factor;
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

static const float3 sampleOffsetDirections[20] = float3[] (
    float3( 1,  1,  1), float3( 1, -1,  1), float3(-1, -1,  1), float3(-1,  1,  1),
    float3( 1,  1, -1), float3( 1, -1, -1), float3(-1, -1, -1), float3(-1,  1, -1),
    float3( 1,  1,  0), float3( 1, -1,  0), float3(-1, -1,  0), float3(-1,  1,  0),
    float3( 1,  0,  1), float3(-1,  0,  1), float3( 1,  0, -1), float3(-1,  0, -1),
    float3( 0,  1,  1), float3( 0, -1,  1), float3( 0, -1, -1), float3( 0,  1, -1)
);

float3 shadowFactorCubemap(Light light, float3 worldPos) {
    const float3 fragToLight = worldPos - light.position.xyz;
    const float currentDepth = length(fragToLight);
    const float3 dir = normalize(fragToLight);
    const float viewDistance = length(scene.cameraPosition - worldPos);
    const float diskRadius   = (1.0 + (viewDistance / light.range)) / 25.0;
    const float bias = 0.15;
    const int samples = 20;
    float shadow = 0.0;

    [unroll]
    for (int i = 0; i < samples; i++)     {
        float3 offsetDir = normalize(fragToLight + sampleOffsetDirections[i] * diskRadius);
        SampledCube sc = sampleCube(offsetDir);
        float sampledDepth = shadowMaps[light.mapIndex + sc.faceIndex]
            .Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR_NOANI], sc.uv).r;
        sampledDepth *= light.range;
        shadow += (currentDepth - bias) > sampledDepth ? SHADOW_FACTOR : 1.0;
    }

    float3 factor = float3(shadow /= samples);
    SampledCube sc = sampleCube(dir);
    const float4 transparencyColor = shadowTransparencyColorMaps[light.mapIndex + sc.faceIndex]
            .Sample(samplers[SAMPLER_LINEAR_LINEAR_EDGE_LINEAR_NOANI], sc.uv);
    if (transparencyColor.a != 1.0) {
        return lerp(transparencyColor.rgb, float3(1.0), factor);
    }
    return factor;
}
