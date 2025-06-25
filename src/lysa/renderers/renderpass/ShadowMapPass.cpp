/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.shadow_map_pass;

import lysa.application;
import lysa.global;
import lysa.resources;
import lysa.virtual_fs;
import lysa.nodes.directional_light;
import lysa.nodes.omni_light;
import lysa.nodes.spot_light;
import lysa.renderers.renderer;

namespace lysa {
    ShadowMapPass::ShadowMapPass(
        const RenderingConfiguration& config,
        const std::shared_ptr<Light>& light):
        Renderpass{config, L"ShaderMaterialPass"},
        light{light} {
        const auto& vireo = Application::getVireo();

        descriptorLayout = vireo.createDescriptorLayout();
        descriptorLayout->add(BINDING_GLOBAL, vireo::DescriptorType::UNIFORM);
        descriptorLayout->build();

        pipelineConfig.resources = vireo.createPipelineResources({
            Resources::descriptorLayout,
            Scene::sceneDescriptorLayout,
            Scene::pipelineDescriptorLayout,
            descriptorLayout},
            Scene::instanceIndexConstantDesc, name);

        pipelineConfig.vertexInputLayout = Application::getVireo().createVertexLayout(sizeof(VertexData), vertexAttributes);
        pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
        pipeline = vireo.createGraphicPipeline(pipelineConfig, name);

        framesData.resize(config.framesInFlight);
        for (auto& frameData : framesData) {
            for (int i = 0; i < 1; i++) {
                frameData.globalUniformBuffer[i] = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(GlobalUniform));
                frameData.globalUniformBuffer[i]->map();
            }
            frameData.descriptorSet = vireo.createDescriptorSet(descriptorLayout);
            frameData.shadowMap = vireo.createRenderTarget(
                pipelineConfig.depthStencilImageFormat,
                1024, 1024,
                vireo::RenderTargetType::DEPTH,
                {}, 1);
        }
    }

    void ShadowMapPass::update(const uint32 frameIndex) {
        auto& data = framesData[frameIndex];
        if (!light->isVisible() || data.currentCamera == nullptr) { return; }
        switch (light->getLightType()) {
            case Light::LIGHT_DIRECTIONAL: {
                throw Exception{"Directional light not supported"};
                break;
            }
            case Light::LIGHT_OMNI: {
                throw Exception{"Omni light not supported"};
                break;
            }
            case Light::LIGHT_SPOT: {
                const auto spotLight= reinterpret_pointer_cast<SpotLight>(light);
                const auto lightDirection = spotLight->getFrontVector();
                const auto lightPosition= light->getPositionGlobal();
                const auto sceneCenter= lightPosition + lightDirection;
                const auto aspect = data.shadowMap->getImage()->getWidth() / data.shadowMap->getImage()->getHeight();
                const float f = 1.0f / std::tan(radians(float1{spotLight->getFov()}) * 0.5f);
                const auto near = spotLight->getNearClipDistance();
                const auto far = spotLight->getRange();
                const float zRange = near - far;
                const auto lightProjection = float4x4{
                    f / aspect, 0.0f,  0.0f,                         0.0f,
                    0.0f,       f,     0.0f,                         0.0f,
                    0.0f,       0.0f,  (far + near) / zRange,        -1.0f,
                    0.0f,       0.0f,  (2.0f * far * near) / zRange, 0.0f};
                data.globalUniform[0].lightSpace = float4x4::look_at(lightPosition, sceneCenter, AXIS_UP);
                data.globalUniformBuffer[0]->write(&data.globalUniform[0]);
                break;
            }
            default:;
        }
    }

    void ShadowMapPass::render(
        vireo::CommandList& commandList,
        const Scene& scene,
        const uint32 frameIndex) {
        const auto& frameData = framesData[frameIndex];

        frameData.descriptorSet->update(BINDING_GLOBAL, frameData.globalUniformBuffer[0]);

        renderingConfig.depthStencilRenderTarget = frameData.shadowMap;
        commandList.barrier(
            frameData.shadowMap,
            vireo::ResourceState::UNDEFINED,
            vireo::ResourceState::RENDER_TARGET_DEPTH);
        commandList.beginRendering(renderingConfig);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptor(Application::getResources().getDescriptorSet(), 0);
        commandList.bindDescriptor(scene.getDescriptorSet(), 1);
        commandList.bindDescriptor(frameData.descriptorSet, 2);
        scene.drawOpaquesAndShaderMaterialsModels(commandList, 3);
        commandList.endRendering();
        commandList.barrier(
            frameData.shadowMap,
            vireo::ResourceState::RENDER_TARGET_DEPTH,
            vireo::ResourceState::UNDEFINED);
    }
}