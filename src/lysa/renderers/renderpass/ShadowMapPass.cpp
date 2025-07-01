/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.shadow_map_pass;

import lysa.application;
import lysa.global;
import lysa.log;
import lysa.resources;
import lysa.virtual_fs;
import lysa.nodes.directional_light;
import lysa.nodes.omni_light;
import lysa.nodes.spot_light;
import lysa.renderers.renderer;

namespace lysa {
    ShadowMapPass::ShadowMapPass(
        const SceneConfiguration& sceneConfig,
        const RenderingConfiguration& config,
        const std::shared_ptr<Light>& light,
        const DeviceMemoryArray& meshInstancesDataArray):
        Renderpass{config, L"ShadowMapPass"},
        light{light},
        meshInstancesDataArray{meshInstancesDataArray},
        sceneConfig{sceneConfig},
        isCubeMap{light->getLightType() == Light::LIGHT_OMNI} {
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
        if (isCubeMap) {
            pipelineConfig.vertexShader = loadShader(VERTEX_SHADER_CUBEMAP);
            pipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER_CUBEMAP);
        } else {
            pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
        }
        pipeline = vireo.createGraphicPipeline(pipelineConfig, name);

        viewport.width = SHADOWMAP_WIDTH;
        viewport.height = SHADOWMAP_HEIGHT;
        scissors.width = SHADOWMAP_WIDTH;
        scissors.height = SHADOWMAP_HEIGHT;

        subpassesCount = isCubeMap ? 6 : 1;
        for (int i = 0; i < subpassesCount; i++) {
            globalUniformBuffer[i] = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(GlobalUniform));
            globalUniformBuffer[i]->map();
            descriptorSet[i] = vireo.createDescriptorSet(descriptorLayout);
            descriptorSet[i]->update(BINDING_GLOBAL, globalUniformBuffer[i]);
            shadowMap[i] = vireo.createRenderTarget(
                pipelineConfig.depthStencilImageFormat,
                SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                vireo::RenderTargetType::DEPTH);
        }
    }

    void ShadowMapPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        const auto& vireo = Application::getVireo();
        for (const auto& pipelineId: std::views::keys(pipelineIds)) {
            for (int i = 0; i < subpassesCount; i++) {
                if (!frustumCullingPipeline[i].contains(pipelineId)) {
                    frustumCullingPipeline[i][pipelineId] = std::make_unique<FrustumCulling>(false, meshInstancesDataArray);
                    culledDrawCommandsCountBuffer[i][pipelineId] = vireo.createBuffer(
                      vireo::BufferType::READWRITE_STORAGE,
                      sizeof(uint32));
                    culledDrawCommandsBuffer[i][pipelineId] = vireo.createBuffer(
                      vireo::BufferType::READWRITE_STORAGE,
                      sizeof(DrawCommand) * sceneConfig.maxMeshSurfacePerPipeline);
                }
            }
        }
    }

    void ShadowMapPass::compute(
        vireo::CommandList& commandList,
        const std::unordered_map<uint32, std::unique_ptr<Scene::PipelineData>>& pipelinesData) const {
        if (!light->isVisible() || !light->getCastShadows()) { return; }
        for (const auto& [pipelineId, pipelineData] : pipelinesData) {
            for (int i = 0; i < subpassesCount; i++) {
                    frustumCullingPipeline[i].at(pipelineId)->dispatch(
                    commandList,
                    pipelineData->drawCommandsCount,
                    inverse(viewMatrix[i]),
                    projection,
                    *pipelineData->instancesArray.getBuffer(),
                    *pipelineData->drawCommandsBuffer,
                    *culledDrawCommandsBuffer[i].at(pipelineId),
                    *culledDrawCommandsCountBuffer[i].at(pipelineId));
            }
        }
    }

    void ShadowMapPass::update(const uint32 frameIndex) {
        if (!light->isVisible() || !light->getCastShadows()) { return; }
        const auto aspectRatio = static_cast<float>(shadowMap[0]->getImage()->getWidth()) / shadowMap[0]->getImage()->getHeight();
        switch (light->getLightType()) {
            case Light::LIGHT_DIRECTIONAL: {
                throw Exception{"Directional light not supported"};
                break;
            }
            case Light::LIGHT_OMNI: {
                const auto& omniLight = reinterpret_pointer_cast<OmniLight>(light);
                const auto lightPosition= light->getPositionGlobal();
                const auto near = omniLight->getNearClipDistance();
                const auto far = omniLight->getRange();
                {
                    const auto target = lightPosition + AXIS_RIGHT;
                    const auto up = float3{0.0, 1.0, 0.0};
                    viewMatrix[0] = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_LEFT;
                    const auto up = float3{0.0, 1.0, 0.0};
                    viewMatrix[1] = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_UP;
                    const auto up = float3{0.0, 0.0, 1.0};
                    viewMatrix[2] = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_DOWN;
                    const auto up = float3{0.0, 0.0, -1.0};
                    viewMatrix[3] = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_BACK;
                    const auto up = float3{0.0, 1.0, 0.0};
                    viewMatrix[4] = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_FRONT;
                    const auto up = float3{0.0, 1.0, 0.0};
                    viewMatrix[5] = lookAt(lightPosition, target, up);
                }
                projection = perspective(radians(90.0f), aspectRatio, near, far);
                for (int i = 0; i < 6; i++) {
                    globalUniform[i].lightSpace = mul(viewMatrix[i], projection);
                    globalUniform[i].lightPosition = float4(lightPosition, far);
                    globalUniformBuffer[i]->write(&globalUniform[i]);
                }
                break;
            }
            case Light::LIGHT_SPOT: {
                const auto spotLight= reinterpret_pointer_cast<SpotLight>(light);
                const auto lightPosition= light->getPositionGlobal();
                const auto lightDirection = spotLight->getFrontVector();
                const auto target = lightPosition + lightDirection;
                projection = perspective(
                    spotLight->getFov(),
                    aspectRatio,
                    spotLight->getNearClipDistance(),
                    spotLight->getRange());
                viewMatrix[0] = lookAt(lightPosition, target, AXIS_UP);
                globalUniform[0].lightSpace = mul(viewMatrix[0], projection);
                globalUniformBuffer[0]->write(&globalUniform[0]);
                break;
            }
            default:;
        }
    }

    void ShadowMapPass::render(
        vireo::CommandList& commandList,
        const Scene& scene) {
        if (!light->isVisible() || !light->getCastShadows()) { return; }


        commandList.setViewport(viewport);
        commandList.setScissors(scissors);

        for (int i = 0; i < subpassesCount; i++) {
            if (firstPass) {
                commandList.barrier(
                  shadowMap[i],
                  vireo::ResourceState::UNDEFINED,
                  vireo::ResourceState::SHADER_READ);
            }

            auto count{0};
            for (const auto& frustumCulling : std::views::values(frustumCullingPipeline[i])) {
                count += frustumCulling->getDrawCommandsCount();
            }
            if (count == 0) {
                continue;
            }

            commandList.barrier(
                shadowMap[i],
                vireo::ResourceState::SHADER_READ,
                vireo::ResourceState::RENDER_TARGET_DEPTH);
            renderingConfig.depthStencilRenderTarget = shadowMap[i];
            commandList.beginRendering(renderingConfig);
            commandList.bindPipeline(pipeline);
            commandList.bindDescriptor(Application::getResources().getDescriptorSet(), SET_RESOURCES);
            commandList.bindDescriptor(scene.getDescriptorSet(), SET_SCENE);
            commandList.bindDescriptor(descriptorSet[i], SET_PASS);
            scene.drawOpaquesAndShaderMaterialsModels(
                commandList,
                SET_PIPELINE,
                culledDrawCommandsBuffer[i],
                culledDrawCommandsCountBuffer[i]);
            commandList.endRendering();
            commandList.barrier(
                shadowMap[i],
                vireo::ResourceState::RENDER_TARGET_DEPTH,
                vireo::ResourceState::SHADER_READ);
        }
        firstPass = false;
    }

}