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
        uint32 modelIndex;
        uint32 surfaceIndex;
    };

    class Scene {
    public:
        static constexpr uint32_t SET_SCENE{2};
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        static constexpr vireo::DescriptorIndex BINDING_MODELS{1};
        static constexpr vireo::DescriptorIndex BINDING_LIGHTS{2};
        inline static std::shared_ptr<vireo::DescriptorLayout> sceneDescriptorLayout{nullptr};

        static constexpr uint32_t SET_PIPELINE{3};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCES_COUNTER{0};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCES{1};
        inline static std::shared_ptr<vireo::DescriptorLayout> pipelineDescriptorLayout{nullptr};

        static void createDescriptorLayouts();
        static void destroyDescriptorLayouts();

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
        void compute(vireo::CommandList& commandList);

        void drawOpaquesModels(
           vireo::CommandList& commandList,
           const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines) const;

        const auto& getMaterials() const { return materials; }
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

        std::list<std::shared_ptr<MeshInstance>> models{};
        DeviceMemoryArray modelsDataArray;
        std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> modelsDataMemoryBlocks{};
        bool modelsDataUpdated{false};

        std::unordered_map<pipeline_id, std::shared_ptr<Material>> materials;
        bool materialsUpdated{false};

        std::list<std::shared_ptr<Light>> lights;
        std::shared_ptr<vireo::Buffer> lightsBuffer;
        uint32 lightsBufferCount{1};

        // FrustumCulling frustumCullingPipeline;

        struct PipelineData {
            pipeline_id pipelineId;
            const SceneConfiguration& config;
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;

            std::shared_ptr<vireo::Buffer> instancesCounterBuffer;

            DeviceMemoryArray instancesArray;
            std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> instancesMemoryBlocks;
            bool instancesUpdated{false};

            std::shared_ptr<vireo::Buffer> drawCommandsBuffer;
            uint32 drawCommandsCount{0};

            PipelineData::PipelineData(
                const SceneConfiguration& config,
                uint32 pipelineId);

            void addNode(
                const std::shared_ptr<MeshInstance>& meshInstance,
                const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& modelsDataMemoryBlocks);

            void removeNode(
                const std::shared_ptr<MeshInstance>& meshInstance);

        };

        std::unordered_map<uint32, std::unique_ptr<PipelineData>> opaquePipelinesData;
    };

}
