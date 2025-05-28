/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
// cf. Samplers.ixx
#define SAMPLER_NEAREST_NEAREST_BORDER_LINEAR s0
#define SAMPLER_LINEAR_LINEAR_EDGE_LINEAR     s1

[[vk::binding(0, 1)]] SamplerState sampler0 : register(SAMPLER_NEAREST_NEAREST_BORDER_LINEAR, space1);
[[vk::binding(1, 1)]] SamplerState sampler1 : register(SAMPLER_LINEAR_LINEAR_EDGE_LINEAR, space1);
