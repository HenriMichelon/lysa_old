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

        viewport.width = SHADOWMAP_WIDTH;
        viewport.height = SHADOWMAP_HEIGHT;
        scissors.width = SHADOWMAP_WIDTH;
        scissors.height = SHADOWMAP_HEIGHT;

        for (int i = 0; i < 1; i++) {
            globalUniformBuffer[i] = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(GlobalUniform));
            globalUniformBuffer[i]->map();
        }
        descriptorSet = vireo.createDescriptorSet(descriptorLayout);
        shadowMap = vireo.createRenderTarget(
            pipelineConfig.depthStencilImageFormat,
            SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
            vireo::RenderTargetType::DEPTH,
            {}, 1);
        renderingConfig.depthStencilRenderTarget = shadowMap;
    }

    void ShadowMapPass::update(const uint32 frameIndex) {
        if (!light->isVisible()) { return; }
        const auto aspectRatio = shadowMap->getImage()->getWidth() / shadowMap->getImage()->getHeight();
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
                const auto lightPosition= light->getPositionGlobal();
                const auto lightDirection = spotLight->getFrontVector();
                const auto target = lightPosition + lightDirection;

                const auto z = normalize(lightPosition - target);
                const auto x = normalize(cross(AXIS_UP, z));
                const auto y = cross(z, x);
                const auto lightView = float4x4{
                    x.x, y.x, z.x, 0,
                    x.y, y.y, z.y, 0,
                    x.z, y.z, z.z, 0,
                    -dot(x, lightPosition), -dot(y, lightPosition), -dot(z, lightPosition), 1
                };

                const auto near = spotLight->getNearClipDistance();
                const auto far = spotLight->getRange();
                const float zRange = near - far;
                const float f = 1.0f / std::tan(spotLight->getFov() * 0.50f);
                const auto lightProjection = float4x4{
                    f/aspectRatio, 0.0f,  0.0f,                          0.0f,
                    0.0f,          f,     0.0f,                          0.0f,
                    0.0f,          0.0f,  (far + near) / zRange,        -1.0f,
                    0.0f,          0.0f,  (2.0f * far * near) / zRange,  0.0f};

                globalUniform[0].lightSpace = mul(lightView, lightProjection);
                globalUniformBuffer[0]->write(&globalUniform[0]);
                break;
            }
            default:;
        }
    }

    void ShadowMapPass::render(
        vireo::CommandList& commandList,
        const Scene& scene) const {
        descriptorSet->update(BINDING_GLOBAL, globalUniformBuffer[0]);

        commandList.barrier(
            shadowMap,
            firstPass ? vireo::ResourceState::UNDEFINED : vireo::ResourceState::SHADER_READ,
            vireo::ResourceState::RENDER_TARGET_DEPTH);
        commandList.setViewport(viewport);
        commandList.setScissors(scissors);
        commandList.bindVertexBuffer(Application::getResources().getVertexArray().getBuffer());
        commandList.bindIndexBuffer(Application::getResources().getIndexArray().getBuffer());

        commandList.beginRendering(renderingConfig);
        commandList.bindPipeline(pipeline);
        commandList.bindDescriptor(Application::getResources().getDescriptorSet(), SET_RESOURCES);
        commandList.bindDescriptor(scene.getDescriptorSet(), SET_SCENE);
        commandList.bindDescriptor(descriptorSet, SET_PASS);
        scene.drawOpaquesAndShaderMaterialsModels(commandList, SET_PIPELINE);
        commandList.endRendering();
        commandList.barrier(
            shadowMap,
            vireo::ResourceState::RENDER_TARGET_DEPTH,
            vireo::ResourceState::SHADER_READ);
    }
}