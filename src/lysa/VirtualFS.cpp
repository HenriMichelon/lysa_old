/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.virtual_fs;

import lysa.application;

namespace lysa {

    std::ifstream VirtualFS::openReadStream(const std::wstring &filepath) {
        std::ifstream file(getPath(filepath), std::ios::binary);
        if (!file.is_open()) { throw Exception("Error: Could not open file ",  to_string(filepath)); }
        return file;
    }

    std::ofstream VirtualFS::openWriteStream(const std::wstring &filepath) {
        std::ofstream file(getPath(filepath), std::ios::binary);
        if (!file.is_open()) { throw Exception("Error: Could not open file ",  to_string(filepath)); }
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

}
