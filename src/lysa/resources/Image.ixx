/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.image;

import std;
import vireo;
import lysa.global;
import lysa.resources.resource;

export namespace lysa {

    /**
     * A bitmap resource, stored in GPU memory.
     */
    class Image : public Resource {
    public:

        /**
         * Returns the width in pixels
         */
        auto getWidth() const { return width; }

        /**
         * Returns the height in pixels
         */
        auto getHeight() const { return height; }

        /**
         * Returns the size in pixels
         */
        auto getSize() const { return float2{getWidth(), getHeight()}; }

        /**
         * Load a bitmap from file.<br>
         * Support JPEG and PNG formats
         */
        // static std::shared_ptr<Image> load(const std::string &filepath, vireo::ImageFormat imageFormat = vireo::ImageFormat::R8G8B8A8_SRGB);

        /**
         * Load a bitmap from memory.<br>
         * Support JPEG & PNG formats.
         */
        // static std::shared_ptr<Image> load(const void* data, uint64_t dataSize, vireo::ImageFormat imageFormat = vireo::ImageFormat::R8G8B8A8_SRGB);

        // static std::shared_ptr<Image> createBlankImage(const Device& device);
        // static std::shared_ptr<Image> createBlankImageArray(const Device& device);

        ~Image() override = default;
    protected:
        uint32 width;
        uint32 height;

        Image(uint32 width, uint32 height, const std::wstring & name);
/*
        static shared_ptr<Image> create(const Device& device,
                                    uint32 width,
                                    uint32 height,
                                    uint64_t imageSize,
                                    const void *data,
                                    const string & name,
                                    ImageFormat format = ImageFormat::R8G8B8A8_SRGB,
                                    bool isArray = false);*/
    };

}
