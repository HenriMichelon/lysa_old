/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <hb.h>
module lysa.renderers.vector;

import lysa.application;
import lysa.constants;
import lysa.exception;
import lysa.log;
import lysa.virtual_fs;

namespace lysa {

    VectorRenderer::VectorRenderer(
        const bool depthTestEnable,
        const bool enableAlphaBlending,
        const bool useTextures,
        const RenderingConfiguration& renderingConfiguration,
        const std::string& name,
        const std::string& shadersName,
        const std::string& glyphShadersName,
        const bool filledTriangles,
        const bool useCamera) :
        config{renderingConfiguration},
        useTextures{useTextures},
        useCamera{useCamera},
        name{name} {
        const auto& vireo = Application::getVireo();

        descriptorLayout = vireo.createDescriptorLayout(name);
        if (useCamera) {
            globalUniformIndex = 0;
            texturesIndex = 1;
            descriptorLayout->add(globalUniformIndex, vireo::DescriptorType::UNIFORM);
        } else {
            texturesIndex = 0;
        }

        if (useTextures) {
            textures.resize(MAX_TEXTURES);
            descriptorLayout->add(texturesIndex, vireo::DescriptorType::SAMPLED_IMAGE, textures.size());
            blankImage = Application::getResources().getBlankImage();
            for (int i = 0; i < textures.size(); i++) {
                textures[i] = blankImage;
            }
        }
        descriptorLayout->build();

        fontsParams.resize(MAX_FONTS);
        fontDescriptorLayout = vireo.createDescriptorLayout(name + " fonts");
        fontDescriptorLayout->add(FONT_PARAMS_BINDING, vireo::DescriptorType::UNIFORM);
        fontDescriptorLayout->build();
        fontDescriptorSet = vireo.createDescriptorSet(fontDescriptorLayout);
        fontsParamsUniform = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(FontParams) * fontsParams.size(), 1, name + " fonts params");
        fontsParamsUniform->map();
        fontsParamsUniform->write(fontsParams.data());
        fontDescriptorSet->update(FONT_PARAMS_BINDING, fontsParamsUniform);

        framesData.resize(config.framesInFlight);
        for (auto& frameData : framesData) {
            frameData.descriptorSet = vireo.createDescriptorSet(descriptorLayout, name);
            if (useCamera) {
                frameData.globalUniform = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(GlobalUniform), 1, name);
                frameData.globalUniform->map();
                frameData.descriptorSet->update(globalUniformIndex, frameData.globalUniform);
            }
            if (useTextures) {
                frameData.descriptorSet->update(texturesIndex, textures);
            }
        }
        renderingConfig.depthTestEnable = depthTestEnable;

        pipelineConfig.colorBlendDesc[0].blendEnable = enableAlphaBlending;
        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.depthTestEnable = depthTestEnable;
        pipelineConfig.depthWriteEnable = depthTestEnable;
        pipelineConfig.colorRenderFormats.push_back(renderingConfiguration.swapChainFormat);
        pipelineConfig.vertexInputLayout = vireo.createVertexLayout(sizeof(Vertex), vertexAttributes);
        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();
        VirtualFS::loadBinaryData("app://" + Application::getConfiguration().shaderDir + "/" + shadersName + ".vert" + ext, tempBuffer);
        pipelineConfig.vertexShader = vireo.createShaderModule(tempBuffer);
        VirtualFS::loadBinaryData("app://" + Application::getConfiguration().shaderDir + "/" + shadersName + ".frag" + ext, tempBuffer);
        pipelineConfig.fragmentShader = vireo.createShaderModule(tempBuffer);
        pipelineConfig.resources = Application::getVireo().createPipelineResources(
           {
               descriptorLayout,
               Application::getResources().getSamplers().getDescriptorLayout(),
                fontDescriptorLayout
           },
           {},
           name);
        pipelineConfig.polygonMode = vireo::PolygonMode::WIREFRAME;
        pipelineConfig.primitiveTopology = vireo::PrimitiveTopology::LINE_LIST;
        pipelineLines = vireo.createGraphicPipeline(pipelineConfig, name + " lines");
        pipelineConfig.polygonMode = filledTriangles ?
            vireo::PolygonMode::FILL :
            vireo::PolygonMode::WIREFRAME;
        pipelineConfig.primitiveTopology = vireo::PrimitiveTopology::TRIANGLE_LIST;
        pipelineTriangles = vireo.createGraphicPipeline(pipelineConfig, name + " triangles");

        VirtualFS::loadBinaryData("app://" + Application::getConfiguration().shaderDir + "/" + glyphShadersName + ".vert" + ext, tempBuffer);
        pipelineConfig.vertexShader = vireo.createShaderModule(tempBuffer);
        VirtualFS::loadBinaryData("app://" + Application::getConfiguration().shaderDir + "/" + glyphShadersName + ".frag" + ext, tempBuffer);
        pipelineConfig.fragmentShader = vireo.createShaderModule(tempBuffer);
        pipelineConfig.polygonMode = vireo::PolygonMode::FILL;
        pipelineConfig.colorBlendDesc = glyphPipelineConfig.colorBlendDesc;
        pipelineGlyphs = vireo.createGraphicPipeline(pipelineConfig, name + " glyphs");
    }

    void VectorRenderer::drawLine(const float3& from, const float3& to, const float4& color) {
        linesVertices.push_back( {from, {}, color});
        linesVertices.push_back( {to, {}, color });
        vertexBufferDirty = true;
    }

    void VectorRenderer::drawTriangle(const float3& v1, const float3& v2, const float3& v3, const float4& color) {
        triangleVertices.push_back( {v1, {}, color});
        triangleVertices.push_back( {v2, {}, color});
        triangleVertices.push_back( {v3, {}, color });
        vertexBufferDirty = true;
    }

    void VectorRenderer::drawText(
        const std::string& text,
        Font& font,
        const float fontScale,
        const float3& position,
        const float4& innerColor) {
        assert([&]{ return useTextures; }, "Can't draw text without textures");
        auto textureIndex = addTexture(font.getAtlas());
        auto fontIndex = addFont(font);
        auto pos = position;

        hb_buffer_t* hb_buffer = hb_buffer_create();
        hb_buffer_add_utf8(hb_buffer, text.c_str(), -1, 0, -1);
        hb_buffer_guess_segment_properties(hb_buffer);
        hb_shape(font.getHarfBuzzFont(), hb_buffer, nullptr, 0);
        unsigned int glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);

        for (unsigned int i = 0; i < glyph_count; i++) {
            auto glyphInfo = font.getGlyphInfo(glyph_info[i].codepoint);
            INFO(glyph_info[i].codepoint);
            auto plane = Font::GlyphBounds{};
            plane.left = fontScale * (glyphInfo.planeBounds.left );
            plane.right = fontScale * (glyphInfo.planeBounds.right );
            plane.top = fontScale * (glyphInfo.planeBounds.top );
            plane.bottom = fontScale * (glyphInfo.planeBounds.bottom);
            /*
            * v1 ---- v3
            * |  \     |
            * |    \   |
            * v0 ---- v2
            */
            const float3 v0 = { pos.x + plane.left,  pos.y + plane.bottom, 0.0f };
            const float3 v1 = { pos.x + plane.left,  pos.y + plane.top, 0.0f };
            const float3 v2 = { pos.x + plane.right, pos.y + plane.bottom, 0.0f };
            const float3 v3 = { pos.x + plane.right, pos.y + plane.top, 0.0f };
            glyphVertices.push_back({v0, {glyphInfo.uv0.x, glyphInfo.uv1.y}, innerColor, {}, textureIndex, fontIndex});
            glyphVertices.push_back({v1, {glyphInfo.uv0.x, glyphInfo.uv0.y}, innerColor, {}, textureIndex, fontIndex});
            glyphVertices.push_back({v2, {glyphInfo.uv1.x, glyphInfo.uv1.y}, innerColor, {}, textureIndex, fontIndex});
            glyphVertices.push_back({v1, {glyphInfo.uv0.x, glyphInfo.uv0.y}, innerColor, {}, textureIndex, fontIndex});
            glyphVertices.push_back({v3, {glyphInfo.uv1.x, glyphInfo.uv0.y}, innerColor, {}, textureIndex, fontIndex});
            glyphVertices.push_back({v2, {glyphInfo.uv1.x, glyphInfo.uv1.y}, innerColor, {}, textureIndex, fontIndex});
            pos.x += fontScale * glyphInfo.advance;
        }

        hb_buffer_destroy(hb_buffer);
        vertexBufferDirty = true;
    }

    void VectorRenderer::restart() {
        linesVertices.clear();
        triangleVertices.clear();
        glyphVertices.clear();
    }

    void VectorRenderer::update(
        const vireo::CommandList& commandList,
        const uint32) {
        // Destroy the previous buffer when we are sure they aren't used by another frame
        oldBuffers.clear();
        const auto& vireo = Application::getVireo();
        if (!linesVertices.empty() || !triangleVertices.empty() || !glyphVertices.empty()) {
            // Resize the buffers only if needed by recreating them
            if ((vertexBuffer == nullptr) ||
                (vertexCount != (linesVertices.size() + triangleVertices.size() + glyphVertices.size()))) {
                // Put the current buffers in the recycle bin since they are currently used
                // and can't be destroyed now
                oldBuffers.push_back(stagingBuffer);
                oldBuffers.push_back(vertexBuffer);
                // Allocate new buffers to change size
                vertexCount = linesVertices.size() + triangleVertices.size() + glyphVertices.size();
                stagingBuffer = vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, sizeof(Vertex), vertexCount, name + " vertices staging");
                stagingBuffer->map();
                vertexBuffer = vireo.createBuffer(vireo::BufferType::VERTEX, sizeof(Vertex), vertexCount, name + " vertices");
                // commandList.barrier(*vertexBuffer, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
            }
            if (vertexBufferDirty) {
                // Push new vertices data to GPU memory
                if (!linesVertices.empty()) {
                    stagingBuffer->write(linesVertices.data(), linesVertices.size() * sizeof(Vertex));
                }
                if (!triangleVertices.empty()) {
                    stagingBuffer->write(
                        triangleVertices.data(),
                        triangleVertices.size() * sizeof(Vertex),
                        linesVertices.size() * sizeof(Vertex));
                }
                if (!glyphVertices.empty()) {
                    stagingBuffer->write(
                        glyphVertices.data(),
                        glyphVertices.size() * sizeof(Vertex),
                        (linesVertices.size() + triangleVertices.size()) * sizeof(Vertex));
                }
                commandList.copy(stagingBuffer, vertexBuffer);
                commandList.barrier(*vertexBuffer, vireo::ResourceState::COPY_DST, vireo::ResourceState::VERTEX_INPUT);
                vertexBufferDirty = false;
            }
        }
    }

    void VectorRenderer::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const uint32 frameIndex) {
        if (vertexCount == 0) {
            return;
        }
        const auto globalUbo = GlobalUniform {
            .projection = scene.getCurrentCamera()->getProjection(),
            .view = inverse(scene.getCurrentCamera()->getTransformGlobal()),
        };
        framesData[frameIndex].globalUniform->write(&globalUbo, sizeof(GlobalUniform));
        render(commandList, colorAttachment, depthAttachment, frameIndex);
    }

    void VectorRenderer::render(
        vireo::CommandList& commandList,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];
        if (vertexCount == 0) {
            return;
        }
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        renderingConfig.depthStencilRenderTarget = depthAttachment;

        commandList.barrier(
            colorAttachment,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.bindVertexBuffer(vertexBuffer);
        commandList.beginRendering(renderingConfig);
        if (!linesVertices.empty()) {
            commandList.bindPipeline(pipelineLines);
            commandList.bindDescriptors({frame.descriptorSet, Application::getResources().getSamplers().getDescriptorSet(), fontDescriptorSet});
            commandList.draw(linesVertices.size(), 1, 0, 0);
        }
        if (!triangleVertices.empty()) {
            commandList.bindPipeline(pipelineTriangles);
            commandList.bindDescriptors({frame.descriptorSet, Application::getResources().getSamplers().getDescriptorSet(), fontDescriptorSet});
            commandList.draw(triangleVertices.size(), 1, linesVertices.size(), 0);
        }
        if (!glyphVertices.empty()) {
            commandList.bindPipeline(pipelineGlyphs);
            commandList.bindDescriptors({frame.descriptorSet, Application::getResources().getSamplers().getDescriptorSet(), fontDescriptorSet});
            commandList.draw(glyphVertices.size(), 1, linesVertices.size() + triangleVertices.size(), 0);
        }
        commandList.endRendering();
        commandList.barrier(
            colorAttachment,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::UNDEFINED);
    }

    int32 VectorRenderer::addTexture(const std::shared_ptr<Image> &texture) {
        if (texturesIndices.contains(texture->getId())) {
            return texturesIndices.at(texture->getId());
        }
        for (int index = 0; index < textures.size(); index++) {
            if (textures[index] == blankImage) {
                textures[index] = texture->getImage();
                texturesIndices[texture->getId()] = index;
                for (const auto& frameData : framesData) {
                    frameData.descriptorSet->update(texturesIndex, textures);
                }
                return index;
            }
        }
        throw Exception("Maximum images count reached for the vector renderer");
    }

    int32 VectorRenderer::addFont(const Font &font) {
        if (fontsIndices.contains(font.getId())) {
            return fontsIndices.at(font.getId());
        }
        for (int index = 0; index < fontsParams.size(); index++) {
            if (all(fontsParams[index].pxRange == FLOAT2ZERO)) {
                fontsParams[index] = font.getFontParams();
                fontsIndices[font.getId()] = index;
                fontsParamsUniform->write(
                    fontsParams.data(),
                    sizeof(FontParams),
                    sizeof(FontParams) * index);
                return index;
            }
        }
        throw Exception("Maximum font count reached for the vector renderer");
    }

}