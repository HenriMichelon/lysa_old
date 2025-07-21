/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.vector;

import lysa.application;
import lysa.virtual_fs;

namespace lysa {

    VectorRenderer::VectorRenderer(
        const bool depthTestEnable,
        const RenderingConfiguration& renderingConfiguration,
        const std::string& name,
        const std::string& shadersName,
        const bool filledTriangles,
        const bool enableAlphaBlending,
        const bool useCamera,
        const bool useTextures) :
        config{renderingConfiguration},
        useCamera{useCamera},
        useTextures{useTextures},
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
           { descriptorLayout, Application::getResources().getSamplers().getDescriptorLayout() },
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
    }

    void VectorRenderer::drawLine(const float3& from, const float3& to, const float4& color) {
        linesVertices.push_back( {from, {}, color, {}, -1 });
        linesVertices.push_back( {to, {}, color, {}, -1 });
        vertexBufferDirty = true;
    }

    void VectorRenderer::drawTriangle(const float3& v1, const float3& v2, const float3& v3, const float4& color) {
        triangleVertices.push_back( {v1, {}, color, {}, -1 });
        triangleVertices.push_back( {v2, {}, color, {}, -1 });
        triangleVertices.push_back( {v3, {}, color, {}, -1 });
        vertexBufferDirty = true;
    }

    void VectorRenderer::restart() {
        linesVertices.clear();
        triangleVertices.clear();
    }

    void VectorRenderer::update(
        const vireo::CommandList& commandList,
        const uint32) {
        // Destroy the previous buffer when we are sure they aren't used by another frame
        oldBuffers.clear();
        const auto& vireo = Application::getVireo();
        if (!linesVertices.empty() || !triangleVertices.empty()) {
            // Resize the buffers only if needed by recreating them
            if ((vertexBuffer == nullptr) || (vertexCount != (linesVertices.size() + triangleVertices.size()))) {
                // Put the current buffers in the recycle bin since they are currently used
                // and can't be destroyed now
                oldBuffers.push_back(stagingBuffer);
                oldBuffers.push_back(vertexBuffer);
                // Allocate new buffers to change size
                vertexCount = linesVertices.size() + triangleVertices.size();
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
                    stagingBuffer->write(triangleVertices.data(), triangleVertices.size() * sizeof(Vertex), linesVertices.size() * sizeof(Vertex));
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
            commandList.bindDescriptors({frame.descriptorSet, Application::getResources().getSamplers().getDescriptorSet()});
            commandList.draw(linesVertices.size(), 1, 0, 0);
        }
        if (!triangleVertices.empty()) {
            commandList.bindPipeline(pipelineTriangles);
            commandList.bindDescriptors({frame.descriptorSet, Application::getResources().getSamplers().getDescriptorSet()});
            commandList.draw(triangleVertices.size(), 1, linesVertices.size(), 0);
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

}