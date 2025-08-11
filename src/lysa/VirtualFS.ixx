/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.virtual_fs;

import vireo;
import lysa.types;

export namespace lysa {

    /**
     * Virtual file system for file path portability
     */
    class VirtualFS {
    public:
        static constexpr auto APP_URI{"app://"};

        static bool dirExists(const std::string &filepath);

        static bool fileExists(const std::string &filepath);

        static std::ifstream openReadStream(const std::string &filepath);

        static std::ofstream openWriteStream(const std::string &filepath);

        static std::string parentPath(const std::string& filepath);

        static void loadBinaryData(const std::string &filepath, std::vector<char>& out);

        static std::byte* loadRGBAImage(const std::string& filepath, uint32& width, uint32& height, uint64& size);

        static void destroyImage(std::byte* image);

        static std::string getPath(const std::string& filepath);
    };

}
