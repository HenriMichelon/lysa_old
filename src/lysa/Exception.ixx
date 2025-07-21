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
export module lysa.exception;

export import std;

export namespace lysa {

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

