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
import lysa.renderers.samplers;

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
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        SceneData(const RenderingConfiguration& config, const vireo::Extent &extent);

        void update() override;

        void draw(
            const std::shared_ptr<vireo::CommandList>& commandList,
            const std::shared_ptr<vireo::Pipeline>& pipeline,
            const Samplers& samplers,
            const std::vector<vireo::DrawIndexedIndirectCommand>& commands,
            const std::shared_ptr<vireo::Buffer>& commandBuffer) const;

        auto getDescriptorSet() const { return descriptorSet; }

    private:
        static constexpr vireo::DescriptorIndex SET_RESOURCES{0};
        static constexpr vireo::DescriptorIndex SET_SCENE{1};
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
    };

}