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
        Image(const std::shared_ptr<vireo::Image>& image, const std::wstring & name);

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

        void save(const std::wstring& filepath) const;
        static void save(const std::wstring& filepath, const std::shared_ptr<vireo::Image>& image);

        ~Image() override = default;

    protected:
        std::shared_ptr<vireo::Image> image;
        uint32 index;
    };

}
