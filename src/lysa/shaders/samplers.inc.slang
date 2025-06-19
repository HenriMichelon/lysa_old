/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
// cf. Samplers.ixx
#define SAMPLER_NEAREST_NEAREST_BORDER_LINEAR 0
#define SAMPLER_LINEAR_LINEAR_EDGE_LINEAR     1
#define SAMPLER_LINEAR_LINEAR_REPEAT_REPEAT   2

[[vk::binding(0, 1)]] SamplerState samplers[20] : register(s0, space1);
