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
        isCubeMap{light->getLightType() == Light::LIGHT_OMNI},
        isCascaded{light->getLightType() == Light::LIGHT_DIRECTIONAL} {
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

        viewport.width = static_cast<float>(light->getShadowMapSize());
        viewport.height = viewport.width;
        scissors.width = light->getShadowMapSize();
        scissors.height = scissors.width;

        if (isCascaded) {
            subpassesCount = reinterpret_pointer_cast<DirectionalLight>(light)->getShadowMapCascadesCount();
            if (subpassesCount < 2 || subpassesCount > 4) {
                throw Exception("Incorrect shadow map cascades count");
            }
        } else {
            subpassesCount = isCubeMap ? 6 : 1;
        }
        subpassData.resize(subpassesCount);
        for (int i = 0; i < subpassesCount; i++) {
            auto& data = subpassData[i];
            data.globalUniformBuffer = vireo.createBuffer(vireo::BufferType::UNIFORM, sizeof(GlobalUniform));
            data.globalUniformBuffer->map();
            data.descriptorSet = vireo.createDescriptorSet(descriptorLayout);
            data.descriptorSet->update(BINDING_GLOBAL, data.globalUniformBuffer);
            int size = light->getShadowMapSize();
            if (isCascaded) {
                size = std::max(512, size >> i);
            }
            data.shadowMap = vireo.createRenderTarget(
                pipelineConfig.depthStencilImageFormat,
                size, size,
                vireo::RenderTargetType::DEPTH,
                renderingConfig.depthStencilClearValue);
            data.transparencyColorMap = vireo.createRenderTarget(
                pipelineConfig.colorRenderFormats[0],
                size, size,
                vireo::RenderTargetType::COLOR,
                renderingConfig.colorRenderTargets[0].clearValue);
        }
    }

    void ShadowMapPass::updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) {
        const auto& vireo = Application::getVireo();
        for (const auto& pipelineId: std::views::keys(pipelineIds)) {
            for (auto& data : subpassData) {
                if (!data.frustumCullingPipelines.contains(pipelineId)) {
                    // INFO("ShadowMapPass::updatePipelines for light ", std::to_string(light->getName()));
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
                    data.inverseViewMatrix,
                    data.projection,
                    *pipelineData->instancesArray.getBuffer(),
                    *pipelineData->drawCommandsBuffer,
                    *data.culledDrawCommandsBuffers.at(pipelineId),
                    *data.culledDrawCommandsCountBuffers.at(pipelineId));
            }
        }
    }

    void ShadowMapPass::update(const uint32) {
        if (!light->isVisible() || !light->getCastShadows()) { return; }
        static constexpr auto aspectRatio{1};
        switch (light->getLightType()) {
            case Light::LIGHT_DIRECTIONAL: {
                auto cascadeSplits = std::vector<float>(subpassesCount);
                const auto& directionalLight = reinterpret_pointer_cast<DirectionalLight>(light);
                const auto& lightDirection = directionalLight->getFrontVector();
                const auto nearClip  = currentCamera->getNearDistance();
                const auto farClip   = currentCamera->getFarDistance();
                const auto clipRange = farClip - nearClip;
                const auto minZ = nearClip;
                const auto maxZ = nearClip + clipRange;
                const auto range = maxZ - minZ;
                const auto ratio = maxZ / minZ;

                // Calculate split depths based on view camera frustum
                // Based on the method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
                for (auto i = 0; i < subpassesCount; i++) {
                    float p          = (i + 1) / static_cast<float>(subpassesCount);
                    float log        = minZ * std::pow(ratio, p);
                    float uniform    = minZ + range * p;
                    float d          = directionalLight->getCascadeSplitLambda() * (log - uniform) + uniform;
                    cascadeSplits[i] = (d - nearClip) / clipRange;
                }

                // Calculate orthographic projection matrix for each cascade
                float lastSplitDist = 0.0;
                const auto invCam = inverse(mul(inverse(currentCamera->getTransformGlobal()), currentCamera->getProjection()));
                for (auto cascadeIndex = 0; cascadeIndex < subpassesCount; cascadeIndex++) {
                    const auto splitDist = cascadeSplits[cascadeIndex];

                    // Camera frustum corners in NDC space
                    float3 frustumCorners[] = {
                        float3(-1.0f, 1.0f, -1.0f),
                        float3(1.0f, 1.0f, -1.0f),
                        float3(1.0f, -1.0f, -1.0f),
                        float3(-1.0f, -1.0f, -1.0f),

                        float3(-1.0f, 1.0f, 1.0f),
                        float3(1.0f, 1.0f, 1.0f),
                        float3(1.0f, -1.0f, 1.0f),
                        float3(-1.0f, -1.0f, 1.0f),
                    };

                    // Camera frustum corners into world space
                    for (auto j = 0; j < 8; j++) {
                        const auto invCorner = mul(float4(frustumCorners[j], 1.0f), invCam);
                        frustumCorners[j]= (invCorner / invCorner.w).xyz;
                    }

                    // Adjust the coordinates of near and far planes for this specific cascade
                    for (auto j = 0; j < 4; j++) {
                        const auto dist = frustumCorners[j + 4] - frustumCorners[j];
                        frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                        frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
                    }

                    // Frustum center for this cascade split, in world space
                    auto frustumCenter = FLOAT3ZERO;
                    for (auto j = 0; j < 8; j++) {
                        frustumCenter += frustumCorners[j];
                    }
                    frustumCenter /= 8.0f;

                    // Radius of the cascade split
                    auto radius = 0.0f;
                    for (auto j = 0; j < 8; j++) {
                        const float distance = length(frustumCorners[j] - frustumCenter);
                        radius = std::max(radius, distance);
                    }
                    radius = std::ceil(radius * 16.0f) / 16.0f;

                    // Snap the frustum center to the nearest texel grid
                    const auto shadowMapResolution = static_cast<float>(subpassData[cascadeIndex].shadowMap->getImage()->getWidth());
                    const float worldUnitsPerTexel = (2.0f * radius) / shadowMapResolution;
                    frustumCenter.x = std::floor(frustumCenter.x / worldUnitsPerTexel) * worldUnitsPerTexel;
                    frustumCenter.y = std::floor(frustumCenter.y / worldUnitsPerTexel) * worldUnitsPerTexel;
                    frustumCenter.z = std::floor(frustumCenter.z / worldUnitsPerTexel) * worldUnitsPerTexel;

                    // Split the bounding box
                    const auto maxExtents = float3(radius);
                    const auto minExtents = -maxExtents;
                    const float depth = maxExtents.z - minExtents.z;

                    // View & projection matrices
                    const auto eye = frustumCenter - lightDirection * -minExtents.z ;
                    const auto viewMatrix = lookAt(eye, frustumCenter, AXIS_UP);
                    auto lightProjection = orthographic(
                        minExtents.x, maxExtents.x,
                        maxExtents.y, minExtents.y,
                        -depth, depth);

                    // https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering
                    // Create the rounding matrix by projecting the world-space origin and determining
                    // the fractional offset in texel space
                    const auto shadowMatrix = mul(viewMatrix, lightProjection);
                    const float4 shadowOrigin =
                        mul(float4(0, 0, 0, 1), shadowMatrix) * (shadowMapResolution * 0.5f);
                    const auto roundedOrigin = round(shadowOrigin);
                    auto roundOffset = roundedOrigin - shadowOrigin;
                    roundOffset = roundOffset * 2.0f / shadowMapResolution;
                    roundOffset.z = 0.0f;
                    roundOffset.w = 0.0f;
                    lightProjection[3] += roundOffset;

                    subpassData[cascadeIndex].inverseViewMatrix = inverse(viewMatrix);
                    subpassData[cascadeIndex].projection = lightProjection;
                    subpassData[cascadeIndex].globalUniform.lightSpace = mul(viewMatrix, lightProjection);
                    subpassData[cascadeIndex].globalUniform.splitDepth = (nearClip + splitDist * clipRange);
                    subpassData[cascadeIndex].globalUniform.transparencyScissor = light->getShadowTransparencyScissors();
                    subpassData[cascadeIndex].globalUniform.transparencyColorScissor = light->getShadowTransparencyColorScissors();
                    subpassData[cascadeIndex].globalUniformBuffer->write(&subpassData[cascadeIndex].globalUniform);
                    lastSplitDist = cascadeSplits[cascadeIndex];
                }
                break;
            }
            case Light::LIGHT_OMNI: {
                const auto lightPosition= light->getPositionGlobal();
                if (any(lastLightPosition != lightPosition)) {
                    const auto& omniLight = reinterpret_pointer_cast<OmniLight>(light);
                    const auto near = omniLight->getNearClipDistance();
                    const auto far = omniLight->getRange();
                    float4x4 viewMatrix[6];
                    viewMatrix[0] = lookAt(
                        lightPosition,
                        lightPosition + AXIS_RIGHT,
                        {0.0, 1.0, 0.0});
                    viewMatrix[1] = lookAt(
                        lightPosition,
                        lightPosition + AXIS_LEFT,
                        {0.0, 1.0, 0.0});
                    viewMatrix[2] = lookAt(
                        lightPosition,
                        lightPosition + AXIS_UP,
                        {0.0, 0.0, 1.0});
                    viewMatrix[3] = lookAt(
                        lightPosition,
                        lightPosition + AXIS_DOWN,
                        {0.0, 0.0, -1.0});
                    viewMatrix[4] = lookAt(
                        lightPosition,
                        lightPosition + AXIS_BACK,
                        {0.0, 1.0, 0.0});
                    viewMatrix[5] = lookAt(
                        lightPosition,
                            lightPosition + AXIS_FRONT,
                            {0.0, 1.0, 0.0});
                    for (int i = 0; i < 6; i++) {
                        subpassData[i].projection = perspective(radians(90.0f), aspectRatio, near, far);
                        subpassData[i].inverseViewMatrix = inverse(viewMatrix[i]);
                        subpassData[i].globalUniform.lightSpace = mul(viewMatrix[i], subpassData[i].projection);
                        subpassData[i].globalUniform.lightPosition = float4(lightPosition, far);
                        subpassData[i].globalUniform.transparencyScissor = light->getShadowTransparencyScissors();
                        subpassData[i].globalUniform.transparencyColorScissor = light->getShadowTransparencyColorScissors();
                        subpassData[i].globalUniformBuffer->write(&subpassData[i].globalUniform);
                    }
                    lastLightPosition = lightPosition;
                }
                break;
            }
            case Light::LIGHT_SPOT: {
                const auto& spotLight= reinterpret_pointer_cast<SpotLight>(light);
                const auto& lightPosition= light->getPositionGlobal();
                const auto& lightDirection = spotLight->getFrontVector();
                const auto target = lightPosition + lightDirection;
                subpassData[0].projection = perspective(
                    spotLight->getFov(),
                    aspectRatio,
                    spotLight->getNearClipDistance(),
                    spotLight->getRange());
                const auto viewMatrix = lookAt(lightPosition, target, AXIS_UP);
                subpassData[0].globalUniform.lightSpace = mul(viewMatrix,  subpassData[0].projection);
                subpassData[0].globalUniform.transparencyScissor = light->getShadowTransparencyScissors();
                subpassData[0].globalUniform.transparencyColorScissor = light->getShadowTransparencyColorScissors();
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
                commandList.barrier(
                  data.transparencyColorMap,
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
            commandList.barrier(
                data.transparencyColorMap,
                vireo::ResourceState::SHADER_READ,
                vireo::ResourceState::RENDER_TARGET_COLOR);
            renderingConfig.depthStencilRenderTarget = data.shadowMap;
            renderingConfig.colorRenderTargets[0].renderTarget = data.transparencyColorMap;
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
            commandList.barrier(
                data.transparencyColorMap,
                vireo::ResourceState::RENDER_TARGET_COLOR,
                vireo::ResourceState::SHADER_READ);
        }
        firstPass = false;
    }

}