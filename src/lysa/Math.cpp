/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.math;

import std;
import lysa.constants;

namespace lysa {

    float4x4 lookAt(const float3& eye, const float3& center, const float3& up) {
        const auto z = normalize(eye - center);
        const auto x = normalize(cross(up, z));
        const auto y = cross(z, x);
        return float4x4{
            x.x, y.x, z.x, 0,
            x.y, y.y, z.y, 0,
            x.z, y.z, z.z, 0,
            -dot(x, eye), -dot(y, eye), -dot(z, eye), 1
        };
    }

    float4x4 perspective(const float fov, const float aspect, const float znear, const float zfar) {
        const float f = 1.0f / std::tan(fov * 0.50f);
        const float zRange = znear - zfar;
        return  float4x4{
            f/aspect, 0.0f,  0.0f,                            0.0f,
            0.0f,     f,     0.0f,                            0.0f,
            0.0f,     0.0f,  (zfar + znear) / zRange,        -1.0f,
            0.0f,     0.0f,  (2.0f * zfar * znear) / zRange,  0.0f};
    }

    float4x4 orthographic(const float left, const float right,
                      const float top, const float  bottom,
                      const float znear, const float zfar) {
        return float4x4{
            2.0f / (right - left),
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            2.0f / (top - bottom),
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            1.0f / (znear - zfar),
            0.0f,

            -(right + left) / (right - left),
            -(top + bottom) / (top - bottom),
            znear / (znear - zfar),
            1.0f
        };
    }

    float3 eulerAngles(quaternion q) {
        q = normalize(q);

        float3 angles;

        // X (pitch)
        float sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
        float cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
        angles.x = std::atan2(sinr_cosp, cosr_cosp);

        // Y (yaw)
        float sinp = 2.0 * (q.w * q.y - q.z * q.x);
        angles.y = (std::abs(sinp) >= 1.0) ? std::copysign(HALF_PI, sinp) : std::asin(sinp);

        // Z (roll)
        float siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
        float cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
        angles.z = std::atan2(siny_cosp, cosy_cosp);

        return angles; // radians
    }
    uint32 randomi(const uint32 max) {
        static std::random_device rd;
        static std::uniform_int_distribution distr(0, static_cast<int>(max));
        std::mt19937 gen(rd());
        return static_cast<uint32>(distr(gen));
    }

    float randomf(const float max) {
        static std::random_device rd;
        static std::uniform_real_distribution<> distr(0, max);
        std::mt19937 gen(rd());
        return static_cast<float>(distr(gen));
    }

}