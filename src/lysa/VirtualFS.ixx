/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.virtual_fs;

import lysa.global;

export namespace lysa {

    /**
     * Virtual file system for file path portability
     */
    class VirtualFS {
    public:
        static constexpr auto APP_URI{L"app://"};

        static bool dirExists(const std::wstring &filepath);

        static bool fileExists(const std::wstring &filepath);

        static std::ifstream openReadStream(const std::wstring &filepath);

        static std::ofstream openWriteStream(const std::wstring &filepath);

        static std::wstring parentPath(const std::wstring& filepath);

        static void loadBinaryData(const std::wstring &filepath, std::vector<char>& out);

    private:
        static std::wstring getPath(const std::wstring& filepath);
    };

}
