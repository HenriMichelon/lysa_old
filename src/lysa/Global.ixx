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

    /**
     * Returns the current time in milliseconds.
     *
     * @return Elapsed time in milliseconds as a floating‑point value.
     */
    float getCurrentTimeMilliseconds();

    /**
     * Produces a sanitized version of a name suitable for identifiers or paths.
     *
     * Typical sanitization removes or replaces characters that are not
     * alphanumeric, underscore, hyphen or dot, and collapses whitespace.
     *
     * @param name Input name to sanitize.
     * @return A sanitized, ASCII‑safe string.
     */
    std::string sanitizeName(const std::string &name);

    /**
     * Tests whether a directory exists at the given path.
     *
     * @param dirName OS path to check.
     * @return True if the directory exists; false otherwise.
     */
    bool dirExists(const std::string& dirName);

    /**
     * Parses a float3 from a textual representation.
     *
     * Expected formats typically include:
     *  - "x y z"
     *  - "x,y,z"
     *
     * @param str Input string containing three floating‑point numbers.
     * @return Parsed float3 value; undefined behavior if the format is invalid.
     */
    float3 to_float3(const std::string& str);

    /**
     * Parses a float4 from a textual representation.
     *
     * Expected formats typically include:
     *  - "x y z w"
     *  - "x,y,z,w"
     *
     * @param str Input string containing four floating‑point numbers.
     * @return Parsed float4 value; undefined behavior if the format is invalid.
     */
    float4 to_float4(const std::string& str);

    /**
     * Splits a string view into sub‑views using a single‑character delimiter.
     *
     * The returned views reference the original memory in str; no allocations
     * are performed for the tokens themselves. Callers must ensure that the
     * lifetime of the original string outlives the returned views.
     *
     * Consecutive delimiters yield empty tokens.
     *
     * @param str       Source string view to split.
     * @param delimiter Delimiter character (e.g., ',').
     * @return Vector of string_view tokens pointing into str.
     */
    std::vector<std::string_view> split(std::string_view str,  char delimiter);

    /**
     * Returns a hexadecimal textual representation of a memory address.
     *
     * @param ptr Pointer value to format.
     * @return Hex string for ptr.
     */
    std::string to_hexstring(const void* ptr);

    /**
     * Returns a hexadecimal textual representation of an integer value.
     *
     * @param ptr Integer value to format in hex.
     * @return Hex string for the input value.
     */
    std::string to_hexstring(uint32 ptr);

    /**
     * Converts a wide‑character C‑string to a UTF‑8 std::string.
     *
     * @param wstr Null‑terminated wide string.
     * @return UTF‑8 encoded string.
     */
    std::string to_string(const wchar_t* wstr);

    /**
     * Returns a human‑readable string for a float3.
     *
     * @param vec Vector to print.
     * @return String representation of vec.
     */
    std::string to_string(const float3& vec);

    /**
     * Returns a human‑readable string for a float2.
     *
     * @param vec Vector to print.
     * @return String representation of vec.
     */
    std::string to_string(const float2& vec);

    /**
     * Returns a human‑readable string for a quaternion (x, y, z, w order).
     *
     * @param vec Quaternion to print.
     * @return String representation of vec.
     */
    std::string to_string(const quaternion& vec);

    /**
     * Returns a human‑readable string for a float4.
     *
     * @param vec Vector to print.
     * @return String representation of vec.
     */
    std::string to_string(const float4& vec);

    /**
     * Converts a UTF‑8 string to UTF‑32 (char32_t) string.
     *
     * @param utf8 Source string encoded in UTF‑8.
     * @return UTF‑32 string (UCS‑4 code points).
     */
    std::u32string to_utf32(const std::string& utf8);

    /**
     * Returns a lowercase copy of the input ASCII/UTF‑8 string.
     *
     * Locale‑dependent rules may not be applied; intended for identifiers and
     * case‑insensitive comparisons in engine code.
     *
     * @param str Source string.
     * @return Lowercased copy of str.
     */
    std::string to_lower(const std::string& str);

}

