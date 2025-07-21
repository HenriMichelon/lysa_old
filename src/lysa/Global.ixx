/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.global;

import std;
import lysa.math;
import lysa.types;

export namespace lysa {

    float getCurrentTimeMilliseconds();

    std::string sanitizeName(const std::string &name);

    bool dirExists(const std::string& dirName);

    float3 to_float3(const std::string& str);

    float4 to_float4(const std::string& str);

    /**
    * Split a string
    */
    std::vector<std::string_view> split(std::string_view str,  char delimiter);

    /**
     * Helper to log a memory address in hexadecimal
     */
    std::string to_hexstring(const void* ptr);

    std::string to_hexstring(uint32 ptr);

    std::string to_string(const wchar_t* wstr);

    /**
     * Helper to log a float3 (std lib code convention)
     */
    std::string to_string(const float3& vec);

    /**
     * Helper to log a float2 (std lib code convention)
     */
    std::string to_string(const float2& vec);

    std::string to_string(const quaternion& vec);

    /**
     * Helper to log a vec4 (std lib code convention)
     */
    std::string to_string(const float4& vec);

    std::u32string to_utf32(const std::string& utf8);

    std::string to_lower(const std::string& str);

}

