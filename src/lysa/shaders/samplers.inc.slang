/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
// cf. Samplers.ixx
static const int SAMPLER_NEAREST_NEAREST_BORDER_LINEAR   = 0;
static const int SAMPLER_LINEAR_LINEAR_EDGE_LINEAR       = 1;
static const int SAMPLER_LINEAR_LINEAR_EDGE_LINEAR_NOANI = 2;
static const int SAMPLER_LINEAR_LINEAR_REPEAT_REPEAT     = 3;
static const int SAMPLER_NEAREST_NEAREST_REPEAT_REPEAT   = 4;
// static const int SAMPLER_NEAREST_NEAREST_BORDER_LINEAR_COMPARE = 3;

[[vk::binding(0, 1)]] SamplerState samplers[20] : register(s0, space1);
