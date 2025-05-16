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

    struct ModelUniform {
        alignas(16) float4x4 transform;
    };

    struct MaterialUniform {
        alignas(16) float4 albedoColor{0.9f, 0.5f, 0.6f, 1.0f};
        alignas(16) float shininess{128.f};
        alignas(4)  int32 diffuseTextureIndex{-1};
    };

    struct PushConstants {
        uint32 modelIndex{0};
        uint32 materialIndex{0};
    };

    class SceneData : public Scene {
    public:
        static constexpr auto pushConstantsDesc = vireo::PushConstantsDesc {
            .stage = vireo::ShaderStage::ALL,
            .size = sizeof(PushConstants),
        };

        SceneData(const std::shared_ptr<vireo::Vireo>& vireo, uint32 framesInFlight, const vireo::Extent &extent);

        void update() override;

        void draw(
            const std::shared_ptr<vireo::CommandList>& commandList,
            const std::shared_ptr<vireo::PipelineResources>& pipelineResources,
            std::unordered_map<BufferMaterialPair, std::vector<vireo::DrawIndexedIndirectCommand>>& drawCommands) const;

        static auto& getDescriptorLayout() { return descriptorLayout; }

        auto getDescriptorSet() const { return descriptorSet; }

    private:
        static constexpr vireo::DescriptorIndex BINDING_SCENE{0};
        static constexpr vireo::DescriptorIndex BINDING_MODELS{1};
        static constexpr vireo::DescriptorIndex BINDING_MATERIALS{2};

        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        std::shared_ptr<vireo::Vireo>            vireo;
        std::shared_ptr<vireo::DescriptorSet>    descriptorSet;

        // Global shader data for the scene
        SceneUniform sceneUniform{};
        // Scene data buffer
        std::shared_ptr<vireo::Buffer> sceneUniformBuffer;

        // Model shader data for all the models of the scene, one array all the models
        std::shared_ptr<ModelUniform[]> modelUniforms;
        uint32 modelUniformsSize{0};
        // Data buffer for all the models of the scene, one buffer for all the models
        std::shared_ptr<vireo::Buffer> modelUniformBuffers{nullptr};

        // Data for all the materials of the scene, one buffer for all the materials
        std::shared_ptr<vireo::Buffer> materialUniformBuffers;
        uint32 materialUniformsSize{0};


    };

}