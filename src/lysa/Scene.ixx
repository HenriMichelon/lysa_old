/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
export module lysa.scene;

import vireo;
import lysa.global;
import lysa.configuration;
import lysa.memory;
import lysa.nodes.camera;
import lysa.nodes.environment;
import lysa.nodes.light;
import lysa.nodes.mesh_instance;
import lysa.nodes.node;
import lysa.resources.material;
import lysa.resources.mesh;
import lysa.pipelines.frustum_culling;

export namespace lysa {

    struct SceneData {
        float3      cameraPosition;
        alignas(16) float4x4 projection;
        float4x4    view;
        float4x4    viewInverse;
        float4      ambientLight{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + strength
        uint32      lightsCount{0};
    };

    struct alignas(8) InstanceData {
        uint32 meshInstanceIndex;
        uint32 meshSurfaceIndex;
    };

    struct DrawCommand {
        uint32 instanceIndex;
        vireo::DrawIndexedIndirectCommand command;
    };

    class Scene {
    public:
        static constexpr uint32_t SET_SCENE{2};
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        static constexpr vireo::DescriptorIndex BINDING_MODELS{1};
        static constexpr vireo::DescriptorIndex BINDING_LIGHTS{2};
        inline static std::shared_ptr<vireo::DescriptorLayout> sceneDescriptorLayout{nullptr};

        static constexpr uint32_t SET_PIPELINE{3};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCES{0};
        inline static std::shared_ptr<vireo::DescriptorLayout> pipelineDescriptorLayout{nullptr};

        static void createDescriptorLayouts();
        static void destroyDescriptorLayouts();

        struct InstanceIndexConstant {
            uint32 instanceIndex;
        };

        static constexpr auto instanceIndexConstantDesc = vireo::PushConstantsDesc {
            .stage = vireo::ShaderStage::VERTEX,
            .size = sizeof(InstanceIndexConstant),
        };


        Scene(
            const SceneConfiguration& config,
            uint32 framesInFlight,
            const vireo::Viewport& viewport,
            const vireo::Rect& scissors);

        auto getCurrentCamera() const { return currentCamera; }

        auto getViewport() const { return viewport; }

        auto getScissors() const { return scissors; }

        virtual void addNode(const std::shared_ptr<Node> &node);

        virtual void removeNode(const std::shared_ptr<Node> &node);

        virtual void activateCamera(const std::shared_ptr<Camera> &camera);

        void update(vireo::CommandList& commandList);

        void compute(vireo::CommandList& commandList) const;

        void setInitialState(vireo::CommandList& commandList) const;

        void drawOpaquesModels(
           vireo::CommandList& commandList,
           const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const;

        void drawTransparentModels(
           vireo::CommandList& commandList,
           const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const;

        const auto& getPipelineIds() const { return pipelineIds; }

        auto isMaterialsUpdated() const { return materialsUpdated; }

        virtual ~Scene() = default;
        Scene(Scene&) = delete;
        Scene& operator=(Scene&) = delete;

    private:
        const SceneConfiguration& config;
        const uint32 framesInFlight;
        const vireo::Viewport& viewport;
        const vireo::Rect& scissors;
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
        std::shared_ptr<Camera> currentCamera{};
        std::shared_ptr<Environment> currentEnvironment{};

        DeviceMemoryArray meshInstancesDataArray;
        std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> meshInstancesDataMemoryBlocks{};
        bool meshInstancesDataUpdated{false};

        std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>> pipelineIds;
        bool materialsUpdated{false};

        std::list<std::shared_ptr<Light>> lights;
        std::shared_ptr<vireo::Buffer> lightsBuffer;
        uint32 lightsBufferCount{1};

        struct PipelineData {
            pipeline_id pipelineId;
            const SceneConfiguration& config;
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;
            FrustumCulling frustumCullingPipeline;

            bool instancesUpdated{false};
            DeviceMemoryArray instancesArray;
            std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> instancesMemoryBlocks;

            uint32 drawCommandsCount{0};
            std::vector<DrawCommand> drawCommands;
            std::shared_ptr<vireo::Buffer> drawCommandsBuffer;
            std::shared_ptr<vireo::Buffer> culledDrawCommandsCountBuffer;
            std::shared_ptr<vireo::Buffer> culledDrawCommandsBuffer;

            PipelineData::PipelineData(
                const SceneConfiguration& config,
                uint32 pipelineId,
                const DeviceMemoryArray& meshInstancesDataArray);

            void addNode(
                const std::shared_ptr<MeshInstance>& meshInstance,
                const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& meshInstancesDataMemoryBlocks);

            void removeNode(
                const std::shared_ptr<MeshInstance>& meshInstance,
                const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& meshInstancesDataMemoryBlocks);

            void addInstance(
                const std::shared_ptr<Mesh>& mesh,
                const MemoryBlock& instanceMemoryBlock,
                const MemoryBlock& meshInstanceMemoryBlock);
        };

        std::unordered_map<uint32, std::unique_ptr<PipelineData>> opaquePipelinesData;
        std::unordered_map<uint32, std::unique_ptr<PipelineData>> transparentPipelinesData;

        void updatePipelineData(
            vireo::CommandList& commandList,
            const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData);

        void compute(
            vireo::CommandList& commandList,
            const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) const;

        void addNode(
            pipeline_id pipelineId,
            const std::shared_ptr<MeshInstance>& meshInstance,
            std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData);

        void drawModels(
           vireo::CommandList& commandList,
           const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines,
           const std::unordered_map<uint32, std::unique_ptr<PipelineData>>& pipelinesData) const;
    };

}
