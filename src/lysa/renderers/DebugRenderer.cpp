/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.debug;

import lysa.application;
import lysa.virtual_fs;

namespace lysa {

    DebugRenderer::DebugRenderer(
        const uint32 framesInFlight,
        const std::wstring& name) :
        name{name} {
        const auto& vireo = Application::getVireo();
        descriptorLayout = vireo.createDescriptorLayout(L"Debug");
        descriptorLayout->add(0, vireo::DescriptorType::UNIFORM);
        descriptorLayout->build();
        framesData.resize(framesInFlight);
        for (auto& frameData : framesData) {
            frameData.globalUniform = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(GlobalUniform), 1, L"Debug global uniform");
            frameData.globalUniform->map();
            frameData.descriptorSet = vireo.createDescriptorSet(descriptorLayout, L"Debug");
            frameData.descriptorSet->update(0, frameData.globalUniform);
        }
        pipelineConfig.vertexInputLayout = vireo.createVertexLayout(sizeof(Vertex), vertexAttributes);
        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();
        VirtualFS::loadBinaryData(L"app://" + Application::getConfiguration().shaderDir + L"/debug.vert" + ext, tempBuffer);
        pipelineConfig.vertexShader = vireo.createShaderModule(tempBuffer);
        VirtualFS::loadBinaryData(L"app://" + Application::getConfiguration().shaderDir + L"/debug.frag" + ext, tempBuffer);
        pipelineConfig.fragmentShader = vireo.createShaderModule(tempBuffer);
        pipelineConfig.resources = Application::getVireo().createPipelineResources(
            { descriptorLayout },
           {},
           L"Debug");
        pipelineConfig.primitiveTopology = vireo::PrimitiveTopology::LINE_LIST;
        pipelineLines = vireo.createGraphicPipeline(pipelineConfig, L"Debug");
        pipelineConfig.primitiveTopology = vireo::PrimitiveTopology::TRIANGLE_LIST;
        pipelineTriangles = vireo.createGraphicPipeline(pipelineConfig, L"Debug");
    }

    void DebugRenderer::drawLine(const float3& from, const float3& to, const float4& color) {
        linesVertices.push_back( {from, color });
        linesVertices.push_back( {to, color });
        vertexBufferDirty = true;
    }

    void DebugRenderer::drawTriangle(const float3& v1, const float3& v2, const float3& v3, const float4& color) {
        triangleVertices.push_back( {v1, color });
        triangleVertices.push_back( {v2, color });
        triangleVertices.push_back( {v3, color });
        vertexBufferDirty = true;
    }

    void DebugRenderer::update(
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
                stagingBuffer = vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, sizeof(Vertex), vertexCount, L"Debug vertices staging");
                stagingBuffer->map();
                vertexBuffer = vireo.createBuffer(vireo::BufferType::VERTEX, sizeof(Vertex), vertexCount, L"Debug vertices");
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

    void DebugRenderer::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
        const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];
        if (vertexCount == 0) { return; }
        const auto globalUbo = GlobalUniform {
            .projection = scene.getCurrentCamera()->getProjection(),
            .view       =  inverse(scene.getCurrentCamera()->getTransformGlobal()),
        };
        frame.globalUniform->write(&globalUbo, sizeof(GlobalUniform));

        // commandList.barrier(
                // colorAttachment,
                // vireo::ResourceState::UNDEFINED,
                // vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.barrier(
                depthAttachment,
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::RENDER_TARGET_DEPTH);
        renderingConfig.colorRenderTargets[0].renderTarget = colorAttachment;
        renderingConfig.depthStencilRenderTarget = depthAttachment;
        commandList.bindVertexBuffer(vertexBuffer);
        commandList.beginRendering(renderingConfig);
        commandList.bindPipeline(pipelineLines);
        commandList.bindDescriptors({frame.descriptorSet});
        if (!linesVertices.empty()) {
            commandList.draw(linesVertices.size(), 1, 0, 0);
        }
        commandList.bindPipeline(pipelineTriangles);
        commandList.bindDescriptors({frame.descriptorSet});
        if (!triangleVertices.empty()) {
            commandList.draw(triangleVertices.size(), 1, linesVertices.size(), 0);
        }
        commandList.endRendering();
        commandList.barrier(
            depthAttachment,
            vireo::ResourceState::RENDER_TARGET_DEPTH,
            vireo::ResourceState::UNDEFINED);
    }

}