/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <stb_image_write.h>
module lysa.resources.font;

import vireo;
import lysa.application;
import lysa.global;
import lysa.log;
import lysa.virtual_fs;
import lysa.window;

namespace lysa {

    void Font::getSize(const std::string &text, float &width, float &height) {
        width = 0;
        uint32 max_height = 0;
        uint32 max_descender = 0;
        for (const auto wc : text) {
            auto &cchar = getFromCache(wc);
            width += static_cast<float>(cchar.advance);
            auto descender = cchar.height - cchar.yBearing;
            max_height = std::max(max_height, cchar.height);
            max_descender = std::max(max_descender, descender);
        }
        height = static_cast<float>(max_height + max_descender);
    }

    std::vector<uint32> Font::renderToBitmap(const std::string &text, float &wwidth, float &hheight) {
        uint32 width = 0;
        uint32 max_height = 0;
        uint32 max_descender = 0;
        const auto utf32 = to_utf32(text);
        for (const auto wc : utf32) {
            auto &cchar = getFromCache(wc);
            width += cchar.advance + cchar.xBearing;
            const auto descender = cchar.height - cchar.yBearing;
            max_height = std::max(max_height, cchar.height);
            max_descender = std::max(max_descender, descender);
        }
        const uint32 height = max_height + max_descender;

        auto bitmap = std::vector<uint32>(width * height, 0);
        int32 x = 0;
        for (const auto wc : utf32) {
            const auto &cchar  = getFromCache(wc);
            const auto offset = height - cchar.yBearing - max_descender;
            for (int line = 0; line < cchar.height; line++) {
                const auto dest = bitmap.data() + (x + cchar.xBearing) + ((line + offset) * width);
                const auto src  = cchar.bitmap->data() + (line * cchar.width);
                for (int col = 0; col < cchar.width; col++) {
                    const auto value = src[col];
                    if (value != 0) { dest[col] = value; }
                }
            }
            x += cchar.advance;
        }
        wwidth  = static_cast<float>(width);
        hheight = static_cast<float>(height);
        return bitmap;
    }

    Font::CachedCharacter &Font::getFromCache(const char32_t c) {
        if (characterCache.contains(c)) {
            return characterCache[c];
        }
        auto &cchar = characterCache[c];
        render(cchar, c);
        return cchar;
    }

    Font::Font(const Font &font, const uint32 size, Window* window) : Font{font.getFontName(), size, window} {
    }

    std::shared_ptr<Image> Font::renderToImage(const std::string &text) {
        if constexpr (isImageCacheEnabled()) {
            if (imageCache.contains(text)) {
                return imageCache[text];
            }
        }
        float width, height;
        const auto bitmap = renderToBitmap(text, width, height);
        const auto image = Image::create(
            bitmap.data(),
            static_cast<uint32>(width),
            static_cast<uint32>(height),
            vireo::ImageFormat::B8G8R8A8_SRGB,
            L"");
        if constexpr (isImageCacheEnabled()) {
            imageCache[text] = image;
        }
        // image->save(L"text.png");
        return image;
    }

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__

    Font::Font(const std::wstring &path, const uint32 size, Window* window) :
        Resource{path},
        path{path},
        size{size},
        window{window} {
        if (window == nullptr) {
            this->window = &Application::getInstance().getMainWindow();
        }
        std::ifstream fontFile = VirtualFS::openReadStream(path);
        fontBuffer = std::make_unique<std::vector<unsigned char>>((std::istreambuf_iterator(fontFile)),
                                                        std::istreambuf_iterator<char>());
        if (!stbtt_InitFont(&font, fontBuffer->data(), stbtt_GetFontOffsetForIndex(fontBuffer->data(), 0))) {
            throw Exception("Failed to initialize font", lysa::to_string(path));
        }
        auto targetHeight = size * this->window->getExtent().height / VECTOR_SCREEN_SIZE;
        scale = stbtt_ScaleForPixelHeight  (&font, targetHeight);

        stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
        height = static_cast<int>(ceilf((ascent - descent) * scale));
        //INFO("Font size : ", size , "->", targetHeight, "=", height, "(", scale, ")");
        ascent  = static_cast<int>(ascent * scale);
        descent = static_cast<int>(descent * scale);
    }

    void Font::render(CachedCharacter &cachedCharacter, const char32_t c) const {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);
        cachedCharacter.advance  = static_cast<int32>(advanceWidth * scale);
        cachedCharacter.xBearing = static_cast<int32>(leftSideBearing * scale);
        int width, height;
        const auto srcBitmap = stbtt_GetCodepointBitmap(
            &font,
            0, scale,
            c,
            &width, &height,
            nullptr, nullptr);
        cachedCharacter.width = width;
        cachedCharacter.height = this->height;
        cachedCharacter.bitmap = std::make_unique<std::vector<uint32>>(cachedCharacter.width * cachedCharacter.height, 0);

        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x1, &y1, &x2, &y2);
        cachedCharacter.yBearing = cachedCharacter.height - (ascent + y1);

        const auto dstBitmap = cachedCharacter.bitmap->data();
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const uint8 gray = srcBitmap[y * width + x];
                if (gray != 0) {
                    dstBitmap[y * cachedCharacter.width + x] =
                        (gray << 24) | (gray << 16) | (gray << 8) | gray;
                }
            }
        }
        // const std::string t = std::format("{}.png", c);
        // stbi_write_png(t.c_str(),
        //        width,
        //        height,
        //        4,
        //        dstBitmap,
        //        width * 4);
        stbtt_FreeBitmap(srcBitmap, nullptr);
    }

    Font::~Font() {
    }


#endif

}
