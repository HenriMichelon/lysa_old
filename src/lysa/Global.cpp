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

}
