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
#ifdef _WIN32
        if (wstr.empty()) return {};
        const int size_needed = WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr.data(),
            static_cast<int>(wstr.size()),
            nullptr, 0, nullptr, nullptr);
        std::string result(size_needed, 0);
        WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr.data(),
            static_cast<int>(wstr.size()),
            result.data(), size_needed,
            nullptr, nullptr);
        return result;
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(wstr);
#endif
    }

    std::wstring to_wstring(const std::string &str) {
#ifdef _WIN32
        const int size_needed = MultiByteToWideChar(
            CP_UTF8,
            0,
            str.data(),
            static_cast<int>(str.size()), nullptr, 0);
        std::wstring result(size_needed, 0);
        MultiByteToWideChar(
            CP_UTF8,
            0,
            str.data(),
            static_cast<int>(str.size()),
            result.data(), size_needed);
        return result;
#else
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
#endif
    }

    std::u32string to_utf32(const std::string& utf8) {
        std::u32string result;
        size_t i = 0;
        while (i < utf8.size()) {
            uint32 cp = 0;
            unsigned char c = utf8[i];
            if (c <= 0x7F) {
                cp = c;
                i += 1;
            } else if ((c & 0xE0) == 0xC0) {
                cp = ((c & 0x1F) << 6) | (utf8[i+1] & 0x3F);
                i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                cp = ((c & 0x0F) << 12) | ((utf8[i+1] & 0x3F) << 6) | (utf8[i+2] & 0x3F);
                i += 3;
            } else if ((c & 0xF8) == 0xF0) {
                cp = ((c & 0x07) << 18) | ((utf8[i+1] & 0x3F) << 12) | ((utf8[i+2] & 0x3F) << 6) | (utf8[i+3] & 0x3F);
                i += 4;
            } else {
                ++i;
                continue;
            }
            result.push_back(cp);
        }
        return result;
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

}