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
import lysa.samplers;
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
            descriptorLayout,
            Application::getResources().getSamplers().getDescriptorLayout()},
            Scene::instanceIndexConstantDesc, name);

        pipelineConfig.vertexInputLayout = Application::getVireo().createVertexLayout(sizeof(VertexData), vertexAttributes);
        pipelineConfig.vertexShader = loadShader(VERTEX_SHADER);
        if (isCubeMap) {
            pipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER_CUBEMAP);
        } else {
            pipelineConfig.fragmentShader = loadShader(FRAGMENT_SHADER);
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
                if (!data.frustumCullingPipelines.contains(pipelineId)) {
                    data.frustumCullingPipelines[pipelineId] = std::make_shared<FrustumCulling>(false, meshInstancesDataArray);
                    data.culledDrawCommandsCountBuffers[pipelineId] = vireo.createBuffer(
                      vireo::BufferType::READWRITE_STORAGE,
                      sizeof(uint32));
                    data.culledDrawCommandsBuffers[pipelineId] = vireo.createBuffer(
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
                data.frustumCullingPipelines.at(pipelineId)->dispatch(
                    commandList,
                    pipelineData->drawCommandsCount,
                    data.viewMatrix,
                    projection,
                    *pipelineData->instancesArray.getBuffer(),
                    *pipelineData->drawCommandsBuffer,
                    *data.culledDrawCommandsBuffers.at(pipelineId),
                    *data.culledDrawCommandsCountBuffers.at(pipelineId));
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
                const auto lightPosition= light->getPositionGlobal();
                if (any(lastLightPosition != lightPosition)) {
                    const auto& omniLight = reinterpret_pointer_cast<OmniLight>(light);
                    const auto near = omniLight->getNearClipDistance();
                    const auto far = omniLight->getRange();
                    subpassData[0].inverseViewMatrix = lookAt(
                        lightPosition,
                        lightPosition + AXIS_RIGHT,
                        {0.0, 1.0, 0.0});
                    subpassData[1].inverseViewMatrix = lookAt(
                        lightPosition,
                        lightPosition + AXIS_LEFT,
                        {0.0, 1.0, 0.0});
                    subpassData[2].inverseViewMatrix = lookAt(
                        lightPosition,
                        lightPosition + AXIS_UP,
                        {0.0, 0.0, 1.0});
                    subpassData[3].inverseViewMatrix = lookAt(
                        lightPosition,
                        lightPosition + AXIS_DOWN,
                        {0.0, 0.0, -1.0});
                    subpassData[4].inverseViewMatrix = lookAt(
                        lightPosition,
                        lightPosition + AXIS_BACK,
                        {0.0, 1.0, 0.0});
                    subpassData[5].inverseViewMatrix = lookAt(
                        lightPosition,
                            lightPosition + AXIS_FRONT,
                            {0.0, 1.0, 0.0});
                    projection = perspective(radians(90.0f), aspectRatio, near, far);
                    for (auto& data : subpassData) {
                        data.viewMatrix = inverse(data.inverseViewMatrix);
                        data.globalUniform.lightSpace = mul(data.inverseViewMatrix, projection);
                        data.globalUniform.lightPosition = float4(lightPosition, far);
                        data.globalUniformBuffer->write(&data.globalUniform);
                    }
                    lastLightPosition = lightPosition;
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
                subpassData[0].inverseViewMatrix = lookAt(lightPosition, target, AXIS_UP);
                subpassData[0].globalUniform.lightSpace = mul(subpassData[0].inverseViewMatrix, projection);
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
            for (const auto& frustumCulling : std::views::values(data.frustumCullingPipelines)) {
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
            commandList.bindDescriptor(Application::getResources().getSamplers().getDescriptorSet(), SET_SAMPLERS);
            scene.drawModels(
                commandList,
                SET_PIPELINE,
                data.culledDrawCommandsBuffers,
                data.culledDrawCommandsCountBuffers,
                data.frustumCullingPipelines);
            commandList.endRendering();
            commandList.barrier(
                data.shadowMap,
                vireo::ResourceState::RENDER_TARGET_DEPTH,
                vireo::ResourceState::SHADER_READ);
        }
        firstPass = false;
    }

}