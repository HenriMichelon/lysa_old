/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
module lysa.virtual_fs;

import lysa.application;

namespace lysa {

    std::ifstream VirtualFS::openReadStream(const std::wstring &filepath) {
        std::ifstream file(getPath(filepath), std::ios::binary);
        if (!file.is_open()) { throw Exception("Error: Could not open file ",  lysa::to_string(filepath)); }
        return file;
    }

    std::ofstream VirtualFS::openWriteStream(const std::wstring &filepath) {
        std::ofstream file(getPath(filepath), std::ios::binary);
        if (!file.is_open()) { throw Exception("Error: Could not open file ",  lysa::to_string(filepath)); }
        return file;
    }

    bool VirtualFS::fileExists(const std::wstring &filepath) {
        return std::filesystem::exists(getPath(filepath));
    }

    std::wstring VirtualFS::parentPath(const std::wstring& filepath) {
        const auto lastSlash = filepath.find_last_of(L"/");
        if (lastSlash == std::string::npos) return L"";
        return filepath.substr(0, lastSlash+1);
    }

    std::wstring VirtualFS::getPath(const std::wstring& filepath) {
        std::wstring filename;
        if (filepath.starts_with(APP_URI)) {
            filename = Application::getConfiguration().appDir;
        } else {
            throw Exception("Unknown URI ", lysa::to_string(filepath));
        }
        const auto filePart = filepath.substr(std::wstring{APP_URI}.size());
        return (filename + L"/" + filePart);
    }

    void VirtualFS::loadBinaryData(const std::wstring &filepath, std::vector<char>& out) {
        std::ifstream file(getPath(filepath), std::ios::ate | std::ios::binary);
        if (!file.is_open()) { throw Exception("failed to open binary file : ", lysa::to_string(filepath)); }
        const size_t fileSize = file.tellg();
        out.clear();
        out.resize(fileSize);
        file.seekg(0);
        file.read(out.data(), fileSize);
        file.close();
    }

    struct StreamWrapper {
        std::ifstream* stream;
    };

    int readCallback(void* user, char* data, const int size) {
        const auto* wrapper = static_cast<StreamWrapper*>(user);
        wrapper->stream->read(data, size);
        return static_cast<int>(wrapper->stream->gcount());
    }

    void skipCallback(void* user, const int n) {
        const auto* wrapper = static_cast<StreamWrapper*>(user);
        wrapper->stream->seekg(n, std::ios::cur);
    }

    int eofCallback(void* user) {
        const auto* wrapper = static_cast<StreamWrapper*>(user);
        return wrapper->stream->eof() ? 1 : 0;
    }

    constexpr auto callbacks = stbi_io_callbacks{
        .read = readCallback,
        .skip = skipCallback,
        .eof  = eofCallback,
    };

    std::byte* VirtualFS::loadRGBAImage(
        const std::wstring& filepath,
        uint32& width, uint32& height, uint64& size) {
        std::ifstream file(getPath(filepath), std::ios::binary);
        if (!file.is_open()) {
            throw Exception ("Error: Could not open file ", lysa::to_string(filepath));
        }

        StreamWrapper wrapper{&file};
        int texWidth, texHeight, texChannels;
        unsigned char* imageData = stbi_load_from_callbacks(
            &callbacks, &wrapper,
            &texWidth, &texHeight,
            &texChannels, STBI_rgb_alpha);
        if (!imageData) {
            throw Exception("Error loading image : ", std::string{stbi_failure_reason()});
        }
        width = static_cast<uint32>(texWidth);
        height = static_cast<uint32>(texHeight);
        size = width * height * STBI_rgb_alpha;
        return reinterpret_cast<std::byte*>(imageData);
    }

    void VirtualFS::destroyImage(std::byte* image) {
        stbi_image_free(image);
    }

}
