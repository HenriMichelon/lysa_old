/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
export module lysa.resources.font;

import std;
import lysa.resources.image;
import lysa.resources.resource;
import lysa.types;

export namespace lysa {

    /**
     * %A font resource to render text
     * %A font is a combination of a font file name and a size.
     */
    class Font : public Resource {
    public:
        friend class Window;

        /**
         * Creates a font resource
         * @param path : font file path, relative to the application working directory
         * @param size : height in pixels on a base resolution of 1920x1080
         * @param window : target window, default to the main window
         */
        Font(const std::string &path, uint32 size, Window* window = nullptr);

        Font(const Font &font, uint32 size, Window* window = nullptr);

        ~Font() override;

        /**
         * Returns the size (in pixels) for a string.
         */
        void getSize(const std::string &text, float &width, float &height);

        /**
         * Returns the font path. Useful to create another Font resource with a different size
         */
        const auto& getFontName() const { return path; }

        /**
         * Returns the font height in pixels (NOT scaled size, but the size given to the Font constructor)
         */
        auto getFontSize() const { return size; }

    private:
        const std::string path;
        const uint32 size;
        Window* window;

    };

}
