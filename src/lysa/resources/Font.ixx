/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <stb_truetype.h>
export module lysa.resources.font;

import std;
import lysa.resources.image;
import lysa.resources.resource;
import lysa.types;

export namespace lysa {

    /**
     * %A font resource to render text in bitmaps.
     * %A font is a combination of a font file name and a size.
     * Supports true type font files (cf https://github.com/nothings/stb/blob/master/stb_truetype.h).
     * The font size is automatically scaled based on the resolution, from a base resolution of 1920x1080
     * (14 is 14 pixels height in this resolution)
     */
    class Font : public Resource {
    public:
        friend class Window;

        /**
         * Creates a font resource
         * @param path : font file path, relative to the application working directory
         * @param size : height in pixels on a base resolution of 1920x1080
         * @param window : target window, default to main window
         */
        Font(const std::wstring &path, uint32 size, Window* window = nullptr);

        Font(const Font &font, uint32 size, Window* window = nullptr);

        ~Font() override;

        /**
         * Returns the size (in pixels) for a string.
         */
        void getSize(const std::string &text, float &width, float &height);

        /**
         *  Renders a string into an RGBA bitmap (stored in CPU memory).
         *  Glyphs are white with alpha channel mapped to the glyphs geometry
         *   @param text : text to render
         *   @param wwidth : width of the resulting bitmap
         *   @param hheight : height of the resulting bitmap
         *   @return 32 bits RGBA bitmap stored in CPU memory
        */
        std::vector<uint32> renderToBitmap(const std::string &text, float &wwidth, float &hheight);

        std::shared_ptr<Image> renderToImage(const std::string &text);

        /**
         * Returns the font path. Useful to create another Font resource with a different size
         */
        const auto& getFontName() const { return path; }

        /**
         * Returns the font height in pixels (NOT scaled size, but the size given to the Font constructor)
         */
        auto getFontSize() const { return size; }

    private:
        // Already rendered characters stored in a cache
        struct CachedCharacter {
            int32 advance;
            int32 xBearing;
            int32 yBearing;
            uint32 width;
            uint32 height;
            std::unique_ptr<std::vector<uint32>> bitmap;
        };

        std::unordered_map<char32_t, CachedCharacter> characterCache;
        const std::wstring path;
        const uint32 size;
        Window* window;

        static constexpr bool ENABLE_IMAGE_CACHE = true;
        static consteval bool isImageCacheEnabled() {
            return ENABLE_IMAGE_CACHE;
        }
        std::unordered_map<std::string, std::shared_ptr<Image>> imageCache;

        CachedCharacter &getFromCache(char32_t c);

        void render(CachedCharacter &, char32_t) const;

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__
        float scale;
        int ascent;
        int descent;
        int lineGap;
        int height;
        stbtt_fontinfo font;
        std::unique_ptr<std::vector<unsigned char>> fontBuffer;
#endif
    };

}
