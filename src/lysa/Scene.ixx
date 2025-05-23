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
import lysa.nodes.viewport;
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
        static constexpr vireo::DescriptorIndex BINDING_VERTEX{0};
        static constexpr vireo::DescriptorIndex BINDING_MATERIAL{1};
        static constexpr vireo::DescriptorIndex BINDING_SCENE{2};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCE_DATA{3};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCE_INDEX{4};
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        Scene(const SceneConfiguration& config, const RenderingConfiguration& rconfig, const vireo::Extent &extent);

        auto getCurrentCamera() const { return currentCamera; }

        auto getViewport() const { return viewport; }

        auto getScissors() const { return scissors; }

        virtual void addNode(const std::shared_ptr<Node> &node);

        virtual void removeNode(const std::shared_ptr<Node> &node);

        virtual void activateCamera(const std::shared_ptr<Camera> &camera);

        void update(vireo::CommandList& commandList);

        void resize(const vireo::Extent &extent);

        void drawOpaquesModels(
           vireo::CommandList& commandList,
           const vireo::Pipeline& pipeline,
           const Samplers& samplers) const;

        virtual ~Scene() = default;
        Scene(Scene&) = delete;
        Scene& operator=(Scene&) = delete;

    private:
        const uint32 framesInFlight;
        vireo::Extent extent;
        vireo::Viewport viewport;
        vireo::Rect scissors;
        std::shared_ptr<Viewport> viewportAndScissors{nullptr};
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
        bool resourcesUpdated{false};

        std::vector<Index> instancesIndex;
        std::shared_ptr<vireo::Buffer> instancesIndexBuffer;
        bool instancesIndexUpdated{false};

        HostVisibleMemoryArray instancesDataArray;
        std::unordered_map<std::shared_ptr<MeshInstance>, MemoryBlock> instancesDataMemoryBlocks{};

        // Currently active camera, first camera added to the scene or the last activated
        std::shared_ptr<Camera> currentCamera{};

        std::list<std::shared_ptr<MeshInstance>> models{};
        std::list<std::shared_ptr<MeshInstance>> opaqueModels{};
        std::shared_ptr<vireo::Buffer> opaqueDrawCommandsBuffer;
        std::shared_ptr<vireo::Buffer> opaqueDrawCommandsStagingBuffer;
        bool commandsUpdated{false};

        void draw(
           vireo::CommandList& commandList,
           const vireo::Pipeline& pipeline,
           const Samplers& samplers,
           const std::shared_ptr<vireo::Buffer>& drawCommand) const;

    };

}
