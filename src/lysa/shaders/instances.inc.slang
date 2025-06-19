/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "scene.inc.slang"

[[vk::binding(0, 3)]] StructuredBuffer<Instance> instances : register(t0, space3);
