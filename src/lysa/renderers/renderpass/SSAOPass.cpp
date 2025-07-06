/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.ssao_pass;

import lysa.application;
import lysa.resources;
import lysa.virtual_fs;
import lysa.resources.mesh;
import lysa.renderers.renderer;

namespace lysa {

    SSAOPass::SSAOPass(
        const RenderingConfiguration& config,
        const GBufferPass& gBufferPass):
        Renderpass{config, L"SSAO"},
        gBufferPass{gBufferPass},
        params{ .radius = config.ssaoRadius, .bias = config.ssaoBias, .power = config.ssaoStrength }{
        const auto& vireo = Application::getVireo();

        descriptorLayout = vireo.createDescriptorLayout();
        descriptorLayout->add(BINDING_PARAMS, vireo::DescriptorType::UNIFORM);
        descriptorLayout->add(BINDING_POSITION_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->add(BINDING_NORMAL_BUFFER, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->add(BINDING_NOISE_TEXTURE, vireo::DescriptorType::SAMPLED_IMAGE);
        descriptorLayout->build();

        pipelineConfig.depthStencilImageFormat = config.depthStencilFormat;
        pipelineConfig.backStencilOpState = pipelineConfig.frontStencilOpState;
        pipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout(),
            Scene::sceneDescriptorLayout,
            descriptorLayout,
            Scene::sceneDescriptorLayoutOptional1},
            {}, name);
        pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
        pipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER);
        pipeline = vireo.createGraphicPipeline(pipelineConfig, name);

        framesData.resize(config.framesInFlight);
        for (auto& frame : framesData) {
            frame.descriptorSet = vireo.createDescriptorSet(descriptorLayout);
        }
    }

    void SSAOPass::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const uint32 frameIndex) {
        const auto& frame = framesData[frameIndex];

        frame.descriptorSet->update(BINDING_POSITION_BUFFER, gBufferPass.getPositionBuffer(frameIndex)->getImage());
        frame.descriptorSet->update(BINDING_NORMAL_BUFFER, gBufferPass.getNormalBuffer(frameIndex)->getImage());

        renderingConfig.colorRenderTargets[0].renderTarget = frame.ssaoColorBuffer;
        commandList.barrier(
           frame.ssaoColorBuffer,
           vireo::ResourceState::SHADER_READ,
           vireo::ResourceState::RENDER_TARGET_COLOR);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptors({
             Application::getResources().getDescriptorSet(),
             Application::getResources().getSamplers().getDescriptorSet(),
             scene.getDescriptorSet(),
             frame.descriptorSet,
             scene.getDescriptorSetOptional1()
        });
        commandList.beginRendering(renderingConfig);
        commandList.setStencilReference(1);
        commandList.draw(3);
        commandList.endRendering();
        commandList.barrier(
            frame.ssaoColorBuffer,
            vireo::ResourceState::RENDER_TARGET_COLOR,
            vireo::ResourceState::SHADER_READ);
    }

    void SSAOPass::resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) {
        const auto& vireo = Application::getVireo();
        for (auto& frame : framesData) {
            frame.ssaoColorBuffer = vireo.createRenderTarget(
                pipelineConfig.colorRenderFormats[0],
                extent.width,extent.height,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[0].clearValue,
                1,
                vireo::MSAA::NONE,
                L"SSAO Color");
            commandList->barrier(
                frame.ssaoColorBuffer,
                vireo::ResourceState::UNDEFINED,
                vireo::ResourceState::SHADER_READ);
        }

        if (paramsBuffer == nullptr) {
            noiseTexture = vireo.createImage(vireo::ImageFormat::R32G32B32A32_SFLOAT, 4, 4, 1, 1, L"SSAO Noise");
            paramsBuffer = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(Params), 1, L"SSAO Params");
            paramsBuffer->map();

            // https://learnopengl.com/Advanced-Lighting/SSAO
            std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
            std::default_random_engine generator;
            for (unsigned int i = 0; i < 64; i++) {
                float3 sample{
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator)
                };
                sample = normalize(sample);
                sample *= randomFloats(generator);
                float scale = static_cast<float>(i) / 64.0;
                scale = std::lerp(0.1f, 1.0f, scale * scale);
                sample *= scale;
                params.samples[i] = float4{sample, 0.0f};
            }

            std::vector<float4> ssaoNoise;
            for (unsigned int i = 0; i < 16; i++){
                float4 noise{
                    randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator) * 2.0 - 1.0,
                    0.0f, 0.0f};
                ssaoNoise.push_back(noise);
            }
            commandList->barrier(noiseTexture, vireo::ResourceState::UNDEFINED, vireo::ResourceState::COPY_DST);
            commandList->upload(noiseTexture, ssaoNoise.data());
            commandList->barrier(noiseTexture, vireo::ResourceState::COPY_DST, vireo::ResourceState::SHADER_READ);

            for (auto& frame : framesData) {
                frame.descriptorSet->update(BINDING_PARAMS, paramsBuffer);
                frame.descriptorSet->update(BINDING_NOISE_TEXTURE, noiseTexture);
            }
        }

        params.screenSize = {extent.width, extent.height};
        params.noiseScale = {extent.width / 4.0f, extent.height / 4.0f};
        paramsBuffer->write(&params);
    }

}