/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#include <windows.h>
#endif
module lysa.global;

namespace lysa {

    std::wstring sanitizeName(const std::wstring &name) {
        auto newName = name;
        std::ranges::replace(newName, '/', '_');
        std::ranges::replace(newName, ':', '_');
        return newName;
    }

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

    bool almostEqual(const float a, const float b, const float epsilon) {
        return std::fabs(a - b) < epsilon;
    }

    bool almostEqual(const float4& a, const float4& b, const float epsilon) {
        return almostEqual(a.x, b.x, epsilon) &&
               almostEqual(a.x, b.y, epsilon) &&
               almostEqual(a.y, b.z, epsilon) &&
               almostEqual(a.z, b.w, epsilon);
    }

    bool almostEqual(const quaternion& a, const quaternion& b, const float epsilon) {
        return almostEqual(a.x, b.x, epsilon) &&
               almostEqual(a.x, b.y, epsilon) &&
               almostEqual(a.y, b.z, epsilon) &&
               almostEqual(a.z, b.w, epsilon);
    }

    float getCurrentTimeMilliseconds() {
        using namespace std::chrono;
        return static_cast<float>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
    }

    std::wstring to_lower(const std::wstring& str) {
        auto s = str;
        // https://en.cppreference.com/w/cpp/string/byte/tolower
        std::ranges::transform(s, s.begin(),
                  [](const unsigned char c){ return std::tolower(c); }
                 );
        return s;
    }

    std::string to_lower(const std::string& str) {
        auto s = str;
        // https://en.cppreference.com/w/cpp/string/byte/tolower
        std::ranges::transform(s, s.begin(),
                  [](const unsigned char c){ return std::tolower(c); }
                 );
        return s;
    }

    std::string to_string(const wchar_t* wstr) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(wstr);
    }

    std::string to_string(const std::wstring& wstr) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(wstr);
    }

    std::wstring to_wstring(const std::string &str) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
    }

    std::vector<std::string_view> split(const std::string_view str, const char delimiter) {
        std::vector<std::string_view> result;
        size_t start = 0;
        size_t end = str.find(delimiter);
        while (end != std::string_view::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }
        result.push_back(str.substr(start)); // Add the last token
        return result;
    }


    float3 to_float3(const std::string& str) {
        std::stringstream ss(str);
        float3 result{};
        if (std::string token; std::getline(ss, token, ',')) {
            result.x = stof(token);
            if (getline(ss, token, ',')) {
                result.y = stof(token);
                if (getline(ss, token, ',')) {
                    result.z = stof(token);
                }
            }
        }
        return result;
    }

    float4 to_float4(const std::string& str) {
        std::stringstream ss(str);
        float4 result{};
        if (std::string token; std::getline(ss, token, ',')) {
            result.x = std::stof(token);
            if (getline(ss, token, ',')) {
                result.y = std::stof(token);
                if (getline(ss, token, ',')) {
                    result.z = std::stof(token);
                    if (getline(ss, token, ',')) {
                        result.w = std::stof(token);
                    }
                }
            }
        }
        return result;
    }

#ifdef _WIN32
    bool dirExists(const std::string& dirName) {
        const DWORD ftyp = GetFileAttributesA(dirName.c_str());
        return (ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY);
    }
#endif

    std::string to_hexstring(const void* ptr) {
        std::stringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<uint64>(ptr);
        return ss.str();
    }

    std::string to_hexstring(const uint32 ptr) {
        std::stringstream ss;
        ss << "0x" << std::hex << ptr;
        return ss.str();
    }

    std::string to_string(const float3& vec) {
        return "{" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "," + std::to_string(vec.z) + "}";
    }

    std::string to_string(const float2& vec) {
        return "{" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "}";
    }

   std::string to_string(const float4& vec) {
        return "{" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "," + std::to_string(vec.z) + std::to_string(vec.w) + "}";
    }

    std::string to_string(const quaternion& vec) {
        return "{" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "," + std::to_string(vec.z) + "," + std::to_string(vec.w) + "}";
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