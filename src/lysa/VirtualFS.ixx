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
     * Virtual file system helper used to resolve portable paths.
     *  - Provide a consistent way to query files/directories and open streams
     *    across platforms and packaging schemes.
     *  - Support a simple URI scheme (app://) to reference assets relative to
     *    the application install or data directory.
     *  - Offer convenience helpers for loading whole files and images.
     *
     * Notes:
     *  - All methods are static; there is no instance to create.
     *  - Thread‑safety: methods are stateless and may be called from any thread.
     */
    class VirtualFS {
    public:
        /** URI scheme used to reference files relative to the application root. */
        static constexpr auto APP_URI{"app://"};

        /**
         * Tests whether a directory exists at the given path or URI.
         *
         * @param filepath URI.
         * @return True if the directory exists; false otherwise.
         */
        static bool dirExists(const std::string &filepath);

        /**
         * Tests whether a regular file exists at the given path or URI.
         *
         * @param filepath URI.
         * @return True if the file exists; false otherwise.
         */
        static bool fileExists(const std::string &filepath);

        /**
         * Opens an input stream for reading the file at path or URI.
         *
         * The returned stream is opened in binary mode unless the implementation
         * states otherwise. Callers should check stream.is_open() (or its state)
         * to ensure the file was successfully opened.
         *
         * @param filepath URI.
         * @return std::ifstream positioned at the start of the file.
         */
        static std::ifstream openReadStream(const std::string &filepath);

        /**
         * Opens an output stream for writing the file at path or URI.
         *
         * The returned stream is typically opened in binary mode and may create
         * intermediate directories depending on platform support.
         * Callers should check stream.is_open() (or its state) to ensure the
         * file was successfully opened.
         *
         * @param filepath URI.
         * @return std::ofstream positioned at the start of the file.
         */
        static std::ofstream openWriteStream(const std::string &filepath);

        /**
         * Returns the parent directory of the provided path or URI.
         *
         * For app:// URIs, the parent is computed after resolution.
         * Trailing separators are ignored.
         *
         * @param filepath URI.
         * @return Parent directory path, or an empty string if none.
         */
        static std::string parentPath(const std::string& filepath);

        /**
         * Loads the entire file contents into a byte buffer.
         *
         * @param filepath URI.
         * @param out      Destination buffer; its contents are replaced by the file bytes.
         */
        static void loadBinaryData(const std::string &filepath, std::vector<char>& out);

        /**
         * Loads an image and returns an allocated RGBA (8‑bit per channel) buffer.
         *
         * The caller owns the returned memory and must release it with destroyImage().
         *
         * @param filepath URI.
         * @param width    Output image width in pixels.
         * @param height   Output image height in pixels.
         * @param size     Output total buffer size in bytes (width*height*4).
         * @return Pointer to the newly allocated pixel buffer in RGBA8888 format, or nullptr on failure.
         */
        static std::byte* loadRGBAImage(const std::string& filepath, uint32& width, uint32& height, uint64& size);

        /**
         * Frees an image buffer allocated by loadRGBAImage().
         *
         * @param image Pointer previously returned by loadRGBAImage() (may be nullptr).
         */
        static void destroyImage(std::byte* image);

        /**
         * Resolves a path or app:// URI to a concrete OS path.
         *
         * Implementations may expand environment variables, normalize separators,
         * and map app:// to the application data/assets directory.
         *
         * @param filepath URI.
         * @return Resolved OS path suitable for file I/O APIs.
         */
        static std::string getPath(const std::string& filepath);
    };

}
