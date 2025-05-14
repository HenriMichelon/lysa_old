/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.global;

import std;
export import lysa.constants;
export import lysa.enums;
export import lysa.math;
export import lysa.object;

export namespace lysa {

    float3 eulerAngles(quaternion q);
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
}
