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
import lysa.samplers;
import lysa.nodes.camera;
import lysa.nodes.mesh_instance;
import lysa.nodes.node;
import lysa.resources.material;

export namespace lysa {

    struct SceneData {
        float3      cameraPosition;
        alignas(16) float4x4 projection;
        float4x4    view;
        float4x4    viewInverse;
        float4      ambientLight{1.0f, 1.0f, 1.0f, 0.01f}; // RGB + strength
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
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCE_DATA{1};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCE_INDEX{2};
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

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

        void update(const vireo::CommandList& commandList);

        void drawOpaquesModels(
           vireo::CommandList& commandList,
           const vireo::Pipeline& pipeline,
           const Samplers& samplers) const;

        virtual ~Scene() = default;
        Scene(Scene&) = delete;
        Scene& operator=(Scene&) = delete;

    private:
        const uint32 framesInFlight;
        const vireo::Viewport& viewport;
        const vireo::Rect& scissors;
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
        // Currently active camera, first camera added to the scene or the last activated
        std::shared_ptr<Camera> currentCamera{};

        std::list<std::shared_ptr<MeshInstance>> models{};
        DeviceMemoryArray instancesDataArray;
        std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> instancesDataMemoryBlocks{};
        bool instancesDataUpdated{false};

        struct ModelsData {
            std::list<std::shared_ptr<MeshInstance>> opaqueModels{};
            std::shared_ptr<vireo::Buffer> opaqueDrawCommandsBuffer;
            std::shared_ptr<vireo::Buffer> opaqueDrawCommandsStagingBuffer;
            std::vector<Index> opaqueInstancesIndex;
            std::shared_ptr<vireo::Buffer> opaqueInstancesIndexBuffer;
            bool opaqueInstancesIndexUpdated{false};

            ModelsData::ModelsData(const SceneConfiguration& config);

            void update(const vireo::CommandList& commandList);

            void addNode(
                const std::shared_ptr<MeshInstance>& meshInstance,
                DeviceMemoryArray& instancesDataArray,
                std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks);

            void rebuildInstancesIndex(
                const std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock>& instancesDataMemoryBlocks);
        };

        ModelsData modelsData;

        void draw(
           vireo::CommandList& commandList,
           const vireo::Pipeline& pipeline,
           const Samplers& samplers,
           const std::shared_ptr<vireo::Buffer>& drawCommand) const;

    };

}
