/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.global;
#ifdef _WIN32
#include <windows.h>
#endif

export import std;
export import lysa.constants;
export import lysa.enums;
export import lysa.math;
export import lysa.object;
export import lysa.types;

export namespace lysa {

    float radians(const float angle) { return radians(float1{angle}); }

    float getCurrentTimeMilliseconds();

    std::wstring sanitizeName(const std::wstring &name);

    class Exception : public std::exception {
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
