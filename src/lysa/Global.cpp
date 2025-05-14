/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.global;

namespace lysa {

    std::wstring sanitizeName(const std::wstring &name) {
        auto newName = name;
        std::ranges::replace(newName, '/', '_');
        std::ranges::replace(newName, ':', '_');
        return newName;
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


    float getCurrentTimeMilliseconds() {
        using namespace std::chrono;
        return static_cast<float>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
    }

}
