/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.image;

import std;
import vireo;
import lysa.math;
import lysa.types;
import lysa.resources.resource;

export namespace lysa {

    /**
     * A bitmap resource, stored in GPU memory.
     */
    class Image : public Resource {
    public:
        Image(const std::shared_ptr<vireo::Image>& image, const std::string & name);

        /**
         * Returns the width in pixels
         */
        auto getWidth() const { return image->getWidth(); }

        /**
         * Returns the height in pixels
         */
        auto getHeight() const { return image->getHeight(); }

        /**
         * Returns the size in pixels
         */
        auto getSize() const { return float2{getWidth(), getHeight()}; }

        auto getImage() const { return image; }

        auto getIndex() const { return index; }

        void save(const std::string& filepath) const;

        static void save(const std::string& filepath, const std::shared_ptr<vireo::Image>& image);

        /**
        * Load a bitmap from a file.<br>
        * Supports JPEG and PNG formats
        */
        static std::shared_ptr<Image> load(
            const std::string &filepath,
            vireo::ImageFormat imageFormat = vireo::ImageFormat::R8G8B8A8_SRGB);

        /**
         * Load a bitmap from memory.<br>
         * Supports JPEG & PNG formats.
         */
        static std::shared_ptr<Image> create(
            const void* data,
            uint32 width, uint32 height,
            vireo::ImageFormat imageFormat = vireo::ImageFormat::R8G8B8A8_SRGB,
            const std::string& name = "Image");

        ~Image() override = default;

    protected:
        std::shared_ptr<vireo::Image> image;
        uint32 index;
    };

}
