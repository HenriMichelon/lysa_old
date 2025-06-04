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

export namespace lysa {

    struct SceneData {
        float3      cameraPosition;
        alignas(16) float4x4 projection;
        float4x4    view;
        float4x4    viewInverse;
        float4      ambientLight{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + strength
        uint32      lightsCount{0};
    };

    struct MeshSurfaceInstanceData {
        float4x4 transform;
        uint32 vertexIndex{0};
        uint32 materialIndex{0};
    };
    static_assert(sizeof(MeshSurfaceInstanceData) == 80, "MeshSurfaceInstanceData must be 80 bytes for StructuredBuffer alignment");

    struct alignas(8) Index {
        uint index;
        uint surfaceIndex;
    };
    static_assert(sizeof(Index) == 8, "Index must be 8 bytes for StructuredBuffer alignment");

    class Scene {
    public:
        static constexpr uint32_t SET_SCENE{2};
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCE_DATA{1};
        static constexpr vireo::DescriptorIndex BINDING_LIGHTS{2};
        inline static std::shared_ptr<vireo::DescriptorLayout> sceneDescriptorLayout{nullptr};

        static constexpr uint32_t SET_DRAW_COMMAND{3};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCE_INDEX{0};
        inline static std::shared_ptr<vireo::DescriptorLayout> drawCommandDescriptorLayout{nullptr};

        static void createDescriptorLayouts();
        static void destroyDescriptorLayouts();

        Scene(
            const SceneConfiguration& config,
            uint32 framesInFlight,
            const vireo::Viewport& viewport,
            const vireo::Rect& scissors);

        auto getViewport() const { return viewport; }

        auto getScissors() const { return scissors; }

        auto getCurrentCamera(const uint32 frameIndex) const { return framesData[frameIndex]->currentCamera; }

        void activateCamera(const std::shared_ptr<Camera> &camera, uint32 frameIndex);

        void addNode(const std::shared_ptr<Node> &node, uint32 frameIndex);

        void removeNode(const std::shared_ptr<Node> &node, uint32 frameIndex);

        void update(const vireo::CommandList& commandList, uint32 frameIndex);

        void drawOpaquesModels(
           vireo::CommandList& commandList,
           const std::unordered_map<uint32, std::shared_ptr<vireo::GraphicPipeline>>& pipelines,
           uint32 frameIndex) const;

        const auto& getMaterials(const uint32 frameIndex) const { return framesData[frameIndex]->materials; }

        auto isMaterialsUpdated(const uint32 frameIndex) const { return framesData[frameIndex]->materialsUpdated; }

        virtual ~Scene() = default;
        Scene(Scene&) = delete;
        Scene& operator=(Scene&) = delete;

    private:
        struct PipelineData {
            uint32 pipelineId;
            std::list<std::shared_ptr<MeshInstance>> models{};
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;

            std::shared_ptr<vireo::Buffer> drawCommandsBuffer;
            std::shared_ptr<vireo::Buffer> drawCommandsStagingBuffer;

            std::vector<Index> instancesIndex;
            std::shared_ptr<vireo::Buffer> instancesIndexBuffer;
            bool instancesIndexUpdated{false};
            const SceneConfiguration& config;

            PipelineData::PipelineData(const SceneConfiguration& config, uint32 pipelineId);

            void update(const vireo::CommandList& commandList);

            void addNode(
                const std::shared_ptr<MeshInstance>& meshInstance,
                DeviceMemoryArray& instancesDataArray,
                std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks);

            void removeNode(
                const std::shared_ptr<MeshInstance>& meshInstance,
                const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks);

            void rebuildInstancesIndex(
                const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks);
        };

        const SceneConfiguration& config;
        const uint32 framesInFlight;
        const vireo::Viewport& viewport;
        const vireo::Rect& scissors;

        struct FrameData {
            const SceneConfiguration& config;
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;
            std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
            std::shared_ptr<Camera> currentCamera{};
            std::shared_ptr<Environment> currentEnvironment{};

            std::list<std::shared_ptr<MeshInstance>> models{};
            DeviceMemoryArray modelsDataArray;
            std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> modelsDataMemoryBlocks{};
            bool modelsDataUpdated{false};

            DeviceMemoryArray instancesDataArray;
            std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> instancesDataMemoryBlocks{};
            bool instancesDataUpdated{false};

            std::unordered_map<uint32, std::shared_ptr<Material>> materials;
            bool materialsUpdated{false};

            std::list<std::shared_ptr<Light>> lights;
            std::shared_ptr<vireo::Buffer> lightsBuffer;
            uint32 lightsBufferCount{1};

            std::unordered_map<uint32, std::unique_ptr<PipelineData>> opaquePipelinesData;

            FrameData(const SceneConfiguration& config);
            void update(const vireo::CommandList& commandList);
            void addNode(const std::shared_ptr<Node> &node);
            void removeNode(const std::shared_ptr<Node> &node);
            void activateCamera(const std::shared_ptr<Camera> &camera);
        };

        std::vector<std::unique_ptr<FrameData>> framesData;
    };

}
