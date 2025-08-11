/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <json.hpp>
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

    void Font::getSize(const std::string &text, float scale, float &width, float &height) {
        width = 0;
        height = 0;
    }

    Font::Font(const std::string &path):
        Resource{path} {
        atlas = Image::load(path + ".png", vireo::ImageFormat::R8G8B8A8_SNORM);

        auto json = nlohmann::ordered_json::parse(VirtualFS::openReadStream(path + ".json"));
        const auto& atlas = json["atlas"];
        atlas["size"].get_to(size);
        uint32 atlasWidth, atlasHeight;
        atlas["width"].get_to(atlasWidth);
        atlas["height"].get_to(atlasHeight);
        auto pixelRange = atlas["distanceRange"].get<float>();
        params.pxRange = { pixelRange / atlasWidth, pixelRange / atlasHeight };

        const auto& metrics = json["metrics"];
        metrics["lineHeight"].get_to(lineHeight);
        metrics["ascender"].get_to(ascender);
        metrics["descender"].get_to(descender);

        for (const auto& glyph : json["glyphs"]) {
            auto glyphInfo = GlyphInfo {
                .codepoint = glyph["unicode"].get<uint32>(),
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
            glyphs[glyphInfo.codepoint] = glyphInfo;
        }
        INFO("Loaded ", glyphs.size(), " glyphs from ", path);
    }

    Font::~Font() {

    }
}
