/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.texture;

import std;
import vireo;
import lysa.math;
import lysa.types;
import lysa.resources.resource;
import lysa.resources.image;

export namespace lysa {

    /**
     * Base class for textures resources.
     */
    class Texture : public Resource {
    public:
        /**
         * Creates a Texture
         * @param name Resource name
         */
        Texture(const std::string &name);

        /**
         * Returns the width in pixels on the texture
         */
        virtual uint32 getWidth() const = 0;

        /**
         * Returns the height in pixels on the texture
         */
        virtual uint32 getHeight() const = 0;

        /**
         * Returns the size in pixels on the texture
         */
        virtual float2 getSize() const { return float2{getWidth(), getHeight()}; }
    };

    /**
     * Image-based texture stored in GPU memory
     */
    class ImageTexture : public Texture {
    public:
        /**
         * Creates an ImageTexture from an existing Image
         */
        ImageTexture(const std::shared_ptr<Image> &image, uint32 samplerIndex);

        /**
         * Returns the attached Image
         */
        const auto& getImage() const { return image; }

        uint32 getWidth() const override { return image->getWidth(); }

        uint32 getHeight() const override { return image->getHeight(); }

        auto getSamplerIndex() const { return samplerIndex; }

    protected:
        std::shared_ptr<Image> image{nullptr};
        uint32 samplerIndex{0};
    };

}
