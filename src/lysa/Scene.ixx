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
        uint32   meshSurfaceIndex;
    };

    class Scene {
    public:
        static constexpr vireo::DescriptorIndex SET_RESOURCES{0};
        static constexpr vireo::DescriptorIndex SET_SCENE{1};
        static constexpr vireo::DescriptorIndex SET_SAMPLERS{2};
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCE_DATA{1};
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        Scene(const RenderingConfiguration& config, const vireo::Extent &extent);

        const auto& getOpaqueModels() const { return opaqueModels; }

        const auto& getOpaqueDrawCommands() const { return opaqueDrawCommands; }

        const auto& getOpaqueDrawCommandsBuffer() const { return opaqueDrawCommandsBuffer; }

        auto getCurrentCamera() const { return currentCamera; }

        auto getViewport() const { return viewport; }

        auto getScissors() const { return scissors; }

        auto getDescriptorSet() const { return descriptorSet; }

        virtual void addNode(const std::shared_ptr<Node> &node);

        virtual void removeNode(const std::shared_ptr<Node> &node);

        virtual void activateCamera(const std::shared_ptr<Camera> &camera);

        void update();

        void resize(const vireo::Extent &extent);

        void draw(
           const std::shared_ptr<vireo::CommandList>& commandList,
           const std::shared_ptr<vireo::Pipeline>& pipeline,
           const Samplers& samplers,
           const std::vector<vireo::DrawIndexedIndirectCommand>& commands,
           const std::shared_ptr<vireo::Buffer>& commandBuffer) const;

        virtual ~Scene() = default;

    private:
        const RenderingConfiguration& config;
        vireo::Extent extent;
        vireo::Viewport viewport;
        vireo::Rect scissors;
        std::shared_ptr<Viewport> viewportAndScissors{nullptr};
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
        bool resourcesUpdated{false};

        std::unique_ptr<MemoryArray> instanceDataArray;
        size_t maxInstanceData{1000};
        // All models
        std::list<std::shared_ptr<MeshInstance>> models{};
        std::vector<MemoryBlock> meshSurfaceMemoryBlocks{};

        // Currently active camera, first camera added to the scene or the last activated
        std::shared_ptr<Camera> currentCamera{};

        // All models containing opaque surfaces
        std::list<std::shared_ptr<MeshInstance>> opaqueModels{};
        std::vector<vireo::DrawIndexedIndirectCommand> opaqueDrawCommands{};
        std::shared_ptr<vireo::Buffer> opaqueDrawCommandsBuffer;
        bool commandsUpdated{false};

        void createInstanceDataArray();

    };

}
