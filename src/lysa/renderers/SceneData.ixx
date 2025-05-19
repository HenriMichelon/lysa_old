/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
export module lysa.renderers.scene_data;

import vireo;
import lysa.global;
import lysa.scene;
import lysa.configuration;
import lysa.nodes.mesh_instance;
import lysa.resources.material;

export namespace lysa {

    struct SceneUniform {
        float3      cameraPosition;
        alignas(16) float4x4 projection;
        float4x4    view;
        float4x4    viewInverse;
        float4      ambientLight{1.0f, 1.0f, 1.0f, 0.01f}; // RGB + strength
    };

    struct MaterialUniform {
        alignas(16) float4 albedoColor{0.9f, 0.5f, 0.6f, 1.0f};
        alignas(16) float shininess{128.f};
        alignas(4)  int32 diffuseTextureIndex{-1};
    };

    struct InstanceData {
        float4x4 transform;
        uint32 materialIndex;
    };

    class SceneData : public Scene {
    public:
        inline static std::shared_ptr<vireo::DescriptorLayout> globalDescriptorLayout{nullptr};
        inline static std::shared_ptr<vireo::DescriptorLayout> perBufferPairDescriptorLayout{nullptr};

        SceneData(const RenderingConfiguration& config, const vireo::Extent &extent);

        void update() override;

        void draw(
            const std::shared_ptr<vireo::CommandList>& commandList,
            const std::shared_ptr<vireo::Pipeline>& pipeline,
            const std::unordered_map<BufferPair, std::vector<vireo::DrawIndexedIndirectCommand>>& commandsByBuffer,
            const std::unordered_map<BufferPair, std::shared_ptr<vireo::Buffer>>& commandsBufferByBuffer) const;

        auto getDescriptorSet() const { return globalDescriptorSet; }

    private:
        static constexpr vireo::DescriptorIndex SET_GLOBAL{0};
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        static constexpr vireo::DescriptorIndex BINDING_MATERIALS{1};
        std::shared_ptr<vireo::DescriptorSet> globalDescriptorSet;
        // Scene data buffer
        std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
        // Data for all the materials of the scene
        std::shared_ptr<vireo::Buffer> materialsStorageBuffer;

        static constexpr vireo::DescriptorIndex SET_PERBUFFER{1};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCES_DATA{0};
        std::unordered_map<BufferPair, std::shared_ptr<vireo::DescriptorSet>> perBufferDescriptorSets;
        std::unordered_map<BufferPair, std::vector<InstanceData>> perBufferInstancesData;
        std::unordered_map<BufferPair, std::shared_ptr<vireo::Buffer>> perBufferInstancesDataBuffer;

    };

}