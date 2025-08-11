/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <json.hpp>
#include <hb.h>
#include <hb-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
module lysa.resources.font;

import vireo;
import lysa.application;
import lysa.constants;
import lysa.exception;
import lysa.global;
import lysa.log;
import lysa.virtual_fs;
import lysa.window;

namespace lysa {

    FT_Library Font::ftLibrary{nullptr};

    void Font::getSize(const std::string &text, float scale, float &width, float &height) {
        width = 0;
        height = 0;
    }

    Font::Font(const std::string &path):
        Resource{path} {
        if (!ftLibrary) {
            if (FT_Init_FreeType(&ftLibrary)) {
                throw Exception("Error initializing FreeType");
            }
        }

        auto json = nlohmann::ordered_json::parse(VirtualFS::openReadStream(path + ".json"));
        const auto& atlas = json["atlas"];
        // assert([&]{ return atlas["type"].get<std::string>() == "mtsdf"; }, "Only MTSDF font atlas are supported");
        atlas["size"].get_to(size);
        uint32 atlasWidth, atlasHeight;
        atlas["width"].get_to(atlasWidth);
        atlas["height"].get_to(atlasHeight);
        const auto pixelRange = atlas["distanceRange"].get<float>();
        params.pxRange = { pixelRange / atlasWidth, pixelRange / atlasHeight };

        if (FT_New_Face(ftLibrary, VirtualFS::getPath(path + ".ttf").c_str(), 0, &ftFace)) {
            if (FT_New_Face(ftLibrary, VirtualFS::getPath(path + ".otf").c_str(), 0, &ftFace)) {
                throw Exception("Error loading font ", path);
            }
        }
        FT_Set_Char_Size(ftFace, 0, size * 64, 0, 0);
        hbFont = hb_ft_font_create(ftFace, nullptr);

        const auto& metrics = json["metrics"];
        metrics["lineHeight"].get_to(lineHeight);
        metrics["ascender"].get_to(ascender);
        metrics["descender"].get_to(descender);

        for (const auto& glyph : json["glyphs"]) {
            auto glyphInfo = GlyphInfo {
                .index = glyph["index"].get<uint32>(),
                .advance = glyph["advance"].get<float>(),
            };
            if (glyph.contains("planeBounds") && glyph.contains("atlasBounds")) {
                glyphInfo.planeBounds.left = glyph["planeBounds"]["left"].get<float>();
                glyphInfo.planeBounds.right = glyph["planeBounds"]["right"].get<float>();
                glyphInfo.planeBounds.top = glyph["planeBounds"]["top"].get<float>();
                glyphInfo.planeBounds.bottom = glyph["planeBounds"]["bottom"].get<float>();

                const auto atlasLeft = glyph["atlasBounds"]["left"].get<float>();
                const auto atlasRight = glyph["atlasBounds"]["right"].get<float>();
                const auto atlasTop = glyph["atlasBounds"]["top"].get<float>();
                const auto atlasBottom = glyph["atlasBounds"]["bottom"].get<float>();
                glyphInfo.uv0 = { atlasLeft / atlasWidth, atlasTop / atlasHeight };
                glyphInfo.uv1 = { atlasRight / atlasWidth, atlasBottom / atlasHeight };
            }
            glyphs[glyphInfo.index] = glyphInfo;
        }
        this->atlas = Image::load(path + ".png", vireo::ImageFormat::R8G8B8A8_SRGB);
        INFO("Loaded ", glyphs.size(), " glyphs from ", path);
    }

    const Font::GlyphInfo& Font::getGlyphInfo(const uint32 index) const {
        if (!glyphs.contains(index)) {
            return glyphs.at(0);
        }
        return glyphs.at(index);
    }

    Font::~Font() {
        hb_font_destroy(hbFont);
        FT_Done_Face(ftFace);
    }
}
