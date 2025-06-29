/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#include <cstdlib>
export module lysa.renderers.renderpass.shadow_map_pass;

import std;
import vireo;
import lysa.configuration;
import lysa.math;
import lysa.memory;
import lysa.scene;
import lysa.nodes.camera;
import lysa.nodes.light;
import lysa.resources.material;
import lysa.resources.mesh;
import lysa.renderers.renderpass;
import lysa.pipelines.frustum_culling;

export namespace lysa {

    class ShadowMapPass : public Renderpass {
    public:
        ShadowMapPass(
            const SceneConfiguration& sceneConfig,
            const RenderingConfiguration& config,
            const std::shared_ptr<Light>& light,
            const DeviceMemoryArray& meshInstancesDataArray);

        void compute(
            vireo::CommandList& commandList,
            const std::unordered_map<uint32, std::unique_ptr<Scene::PipelineData>>& pipelinesData) const;

        void updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds);

        void update(uint32 frameIndex) override;

        void render(
            vireo::CommandList& commandList,
            const Scene& scene);

        auto getShadowMapCount() const { return subpassesCount; }

        auto getShadowMap(const uint32 index) const {
            return shadowMap[index];
        }

        const auto& getLightSpace(const uint32 index) const {
            return globalUniform[index].lightSpace;
        }

    private:
        const std::wstring VERTEX_SHADER{L"shadowmap.vert"};
        const std::wstring VERTEX_SHADER_CUBEMAP{L"shadowmap_cubemap.vert"};
        const std::wstring FRAGMENT_SHADER_CUBEMAP{L"shadowmap_cubemap.frag"};

        static constexpr uint32 SET_RESOURCES{0};
        static constexpr uint32 SET_SCENE{1};
        static constexpr uint32 SET_PIPELINE{2};
        static constexpr uint32 SET_PASS{3};
        static constexpr vireo::DescriptorIndex BINDING_GLOBAL{0};

        static constexpr uint32 SHADOWMAP_WIDTH = 1024;
        static constexpr uint32 SHADOWMAP_HEIGHT = 1024;

        static constexpr uint32 CASCADED_SHADOWMAP_MAX_LAYERS = 4;

        // Lambda constant for split depth calculation :
        // the closer to 1.0 the smaller the firsts splits
        static constexpr auto cascadeSplitLambda = 0.95f;

        struct GlobalUniform {
            float4x4 lightSpace;
            float4   lightPosition; // XYZ: Position, W: far plane
        };

        const bool isCubeMap;
        uint32 subpassesCount;
        float4x4 projection;
        std::shared_ptr<vireo::RenderTarget> shadowMap[6];

        GlobalUniform globalUniform[6];
        std::shared_ptr<vireo::Buffer> globalUniformBuffer[6];
        std::shared_ptr<vireo::DescriptorSet> descriptorSet[6];
        std::map<pipeline_id, std::unique_ptr<FrustumCulling>> frustumCullingPipeline[6];
        std::map<pipeline_id, std::shared_ptr<vireo::Buffer>> culledDrawCommandsBuffer[6];
        std::map<pipeline_id, std::shared_ptr<vireo::Buffer>> culledDrawCommandsCountBuffer[6];

        const std::vector<vireo::VertexAttributeDesc> vertexAttributes {
            {"POSITION", vireo::AttributeFormat::R32G32B32A32_FLOAT, offsetof(VertexData, position)},
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .depthStencilImageFormat = vireo::ImageFormat::D32_SFLOAT,
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthBiasEnable = true,
            .depthBiasConstantFactor = 1.25f,
            .depthBiasSlopeFactor = 1.75f,
        };

        vireo::RenderingConfiguration renderingConfig {
            .depthTestEnable = pipelineConfig.depthTestEnable,
            .clearDepthStencil = true,
            .discardDepthStencilAfterRender = false,
        };

        bool firstPass{true};

        const SceneConfiguration& sceneConfig;
        const DeviceMemoryArray& meshInstancesDataArray;

        vireo::Rect scissors;
        vireo::Viewport viewport;
        std::shared_ptr<Light> light;
        std::shared_ptr<vireo::GraphicPipeline> pipeline;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
    };
}