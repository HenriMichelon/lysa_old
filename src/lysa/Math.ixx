/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cmath>
#include <cstdint>
#include <x86intrin.h>
export module lysa.math;

using int32 = int32_t;
using uint32 = uint32_t;

#define HLSLPP_FEATURE_TRANSFORM
#define HLSLPP_MODULE_DECLARATION

#include "hlsl++/vector_float.h"
#include "hlsl++/vector_float8.h"

#include "hlsl++/vector_int.h"
#include "hlsl++/vector_uint.h"
#include "hlsl++/vector_double.h"

#include "hlsl++/matrix_float.h"

#include "hlsl++/quaternion.h"
#include "hlsl++/dependent.h"

#include "hlsl++/data_packing.h"

export namespace lysa {

    float3 eulerAngles(quaternion q);

    float radians(const float angle) { return radians(float1{angle}); }

    inline bool almostEquals(const float f1, const float f2) {
        return (std::fabs(f1 - f2) <=  0.0001 * std::fmax(std::fabs(f1), std::fabs(f2)));
    }

    inline bool almostEquals(const quaternion& f1, const quaternion& f2) {
        return almostEquals(f1.x, f2.x) &&
            almostEquals(f1.y, f2.y) &&
            almostEquals(f1.z, f2.z) &&
            almostEquals(f1.w, f2.w);
    }

    float4x4 lookAt(const float3& eye, const float3& center, const float3& up);

    float4x4 perspective(float fov, float aspect, float near, float far);

    float4x4 orthographic(float left, float right,
                          float top, float  bottom,
                          float znear, float zfar);

    /**
    * Returns a random value in the range [0, max]
    */
    uint32 randomi(uint32 max);

    /**
    * Returns a random value in the range [0.0f, max]
    */
    float randomf(float max);

}

