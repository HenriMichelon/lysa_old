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

        const auto size = isCubeMap ? config.omniLightShadowMapSize : config.spotLightShadowMapSize;
        viewport.width = static_cast<float>(size);
        viewport.height = viewport.width;
        scissors.width = size;
        scissors.height = size;

        subpassesCount = isCubeMap ? 6 : 1;
        subpassData.resize(subpassesCount);
        for (auto& data : subpassData) {
            data.globalUniformBuffer = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(GlobalUniform));
            data.globalUniformBuffer->map();
            data.descriptorSet = vireo.createDescriptorSet(descriptorLayout);
            data.descriptorSet->update(BINDING_GLOBAL, data.globalUniformBuffer);
            data.shadowMap = vireo.createRenderTarget(
                pipelineConfig.depthStencilImageFormat,
                size, size,
                vireo::RenderTargetType::DEPTH);
        }
    }

    void ShadowMapPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        const auto& vireo = Application::getVireo();
        for (const auto& pipelineId: std::views::keys(pipelineIds)) {
            for (auto& data : subpassData) {
                if (!data.frustumCullingPipeline.contains(pipelineId)) {
                    data.frustumCullingPipeline[pipelineId] = std::make_shared<FrustumCulling>(false, meshInstancesDataArray);
                    data.culledDrawCommandsCountBuffer[pipelineId] = vireo.createBuffer(
                      vireo::BufferType::READWRITE_STORAGE,
                      sizeof(uint32));
                    data.culledDrawCommandsBuffer[pipelineId] = vireo.createBuffer(
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
            for (const auto& data : subpassData) {
                data.frustumCullingPipeline.at(pipelineId)->dispatch(
                    commandList,
                    pipelineData->drawCommandsCount,
                    inverse(data.viewMatrix),
                    projection,
                    *pipelineData->instancesArray.getBuffer(),
                    *pipelineData->drawCommandsBuffer,
                    *data.culledDrawCommandsBuffer.at(pipelineId),
                    *data.culledDrawCommandsCountBuffer.at(pipelineId));
            }
        }
    }

    void ShadowMapPass::update(const uint32 frameIndex) {
        if (!light->isVisible() || !light->getCastShadows()) { return; }
        static constexpr auto aspectRatio{1};
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
                    subpassData[0].viewMatrix = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_LEFT;
                    const auto up = float3{0.0, 1.0, 0.0};
                    subpassData[1].viewMatrix = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_UP;
                    const auto up = float3{0.0, 0.0, 1.0};
                    subpassData[2].viewMatrix = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_DOWN;
                    const auto up = float3{0.0, 0.0, -1.0};
                    subpassData[3].viewMatrix = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_BACK;
                    const auto up = float3{0.0, 1.0, 0.0};
                    subpassData[4].viewMatrix = lookAt(lightPosition, target, up);
                }
                {
                    const auto target = lightPosition + AXIS_FRONT;
                    const auto up = float3{0.0, 1.0, 0.0};
                    subpassData[5].viewMatrix = lookAt(lightPosition, target, up);
                }
                projection = perspective(radians(90.0f), aspectRatio, near, far);
                for (auto& data : subpassData) {
                    data.globalUniform.lightSpace = mul(data.viewMatrix, projection);
                    data.globalUniform.lightPosition = float4(lightPosition, far);
                    data.globalUniformBuffer->write(&data.globalUniform);
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
                subpassData[0].viewMatrix = lookAt(lightPosition, target, AXIS_UP);
                subpassData[0].globalUniform.lightSpace = mul(subpassData[0].viewMatrix, projection);
                subpassData[0].globalUniformBuffer->write(&subpassData[0].globalUniform);
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

        for (const auto& data : subpassData) {
            if (firstPass) {
                commandList.barrier(
                  data.shadowMap,
                  vireo::ResourceState::UNDEFINED,
                  vireo::ResourceState::SHADER_READ);
            }

            auto count{0};
            for (const auto& frustumCulling : std::views::values(data.frustumCullingPipeline)) {
                count += frustumCulling->getDrawCommandsCount();
            }
            if (count == 0) {
                continue;
            }

            commandList.barrier(
                data.shadowMap,
                vireo::ResourceState::SHADER_READ,
                vireo::ResourceState::RENDER_TARGET_DEPTH);
            renderingConfig.depthStencilRenderTarget = data.shadowMap;
            commandList.beginRendering(renderingConfig);
            commandList.bindPipeline(pipeline);
            commandList.bindDescriptor(Application::getResources().getDescriptorSet(), SET_RESOURCES);
            commandList.bindDescriptor(scene.getDescriptorSet(), SET_SCENE);
            commandList.bindDescriptor(data.descriptorSet, SET_PASS);
            scene.drawOpaquesAndShaderMaterialsModels(
                commandList,
                SET_PIPELINE,
                data.culledDrawCommandsBuffer,
                data.culledDrawCommandsCountBuffer);
            commandList.endRendering();
            commandList.barrier(
                data.shadowMap,
                vireo::ResourceState::RENDER_TARGET_DEPTH,
                vireo::ResourceState::SHADER_READ);
        }
        firstPass = false;
    }

}