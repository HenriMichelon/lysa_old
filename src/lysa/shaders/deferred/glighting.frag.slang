/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
#include "deferred.inc.slang"

float4 fragmentMain(QuadOutput input) : SV_TARGET {
    return getColor(input);
}
