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
export module lysa.global;

export import std;
export import lysa.constants;
export import lysa.enums;
export import lysa.math;
export import lysa.object;
export import lysa.types;

export namespace lysa {

    float radians(const float angle) { return radians(float1{angle}); }

    float4x4 lookAt(const float3& eye, const float3& center, const float3& up);

    float4x4 perspective(float fov, float aspect, float near, float far);

    float getCurrentTimeMilliseconds();

    std::wstring sanitizeName(const std::wstring &name);

    bool dirExists(const std::string& dirName);

    float3 to_float3(const std::string& str);

    float4 to_float4(const std::string& str);

    /**
    * Split a string
    */
    std::vector<std::string_view> split(std::string_view str,  char delimiter);

    float3 eulerAngles(quaternion q);

    /**
     * Helper to log a memory address in hexadecimal
     */
    std::string to_hexstring(const void* ptr);
    std::string to_hexstring(uint32 ptr);

    std::string to_string(const wchar_t* wstr);

    std::string to_string(const std::wstring& wstr);

    /**
     * Helper to log a vec3 (std lib code convention)
     */
    std::string to_string(const float3& vec);

    /**
     * Helper to log a vec2 (std lib code convention)
     */
    std::string to_string(const float2& vec);

    std::string to_string(const quaternion& vec);

    /**
     * Helper to log a vec4 (std lib code convention)
     */
    std::string to_string(const float4& vec);

    std::wstring to_wstring(const std::string& str);

    std::wstring to_lower(const std::wstring& str);

    std::string to_lower(const std::string& str);

    /**
    * Returns a random value in the range [0, max]
    */
    uint32 randomi(uint32 max);

    /**
    * Returns a random value in the range [0.0f, max]
    */
    float randomf(float max);

    class Exception final : public std::exception {
    public:
        template <typename... Args>
        explicit Exception(Args&&... args) {
            std::ostringstream oss;
            (oss << ... << std::forward<Args>(args));
#ifdef _MSC_VER
            message = oss.str();
#endif
#ifdef _DEBUG
#ifdef _WIN32
            if (IsDebuggerPresent()) {
                OutputDebugStringA(message.c_str());
#endif
#ifdef __has_builtin
    __builtin_debugtrap();
#endif
#ifdef _MSC_VER
                __debugbreak();
#endif
#ifdef _WIN32
            }
#endif
#endif
        }

        const char* what() const noexcept override {
            return message.c_str();
        }

    private:
        std::string message;
    };


#ifdef _DEBUG

    template<typename Expr>
    constexpr void assert(
        Expr&& expression,
        const std::string message,
        const std::source_location& loc = std::source_location::current()) {
        if (!expression()) {
            throw Exception("Assertion failed: ", message, ", file ", loc.file_name(), ", line ", loc.line());
        }
    }

#else

    template<typename Expr>
    constexpr void assert_expr(
        Expr&&,
        const std::string_view,
        const std::source_location& = std::source_location::current()) noexcept { }

#endif
}

