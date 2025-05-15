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
import lysa.resources.material;

export namespace lysa {

    // struct SceneUniform {
        // float3      cameraPosition;
        // alignas(16) float4x4 projection;
        // float4x4    view;
        // float4x4    viewInverse;
        // float4      ambientLight{1.0f, 1.0f, 1.0f, 0.01f}; // RGB + strength
    // };

    // struct ModelUniform {
        // alignas(16) float4x4 transform;
    // };

    // struct MaterialUniform {
    //     alignas(16) float4 albedoColor{0.9f, 0.5f, 0.6f, 1.0f};
    //     alignas(16) float shininess{128.f};
    //     alignas(4)  int32 diffuseTextureIndex{-1};
    // };

    class SceneData : public Scene {
    public:

    private:
        // Global shader data for the scene
        // SceneUniform sceneUniform{};
        // Model shader data for all the models of the scene, one array all the models
        // std::shared_ptr<ModelUniform[]> modelUniforms;
        // All materials used in the scene, used to update the buffer in GPU memory
        // std::list<std::shared_ptr<Material>> materials;

        // Scene data buffer
        // std::shared_ptr<vireo::Buffer> sceneUniformBuffer;
        // Data buffer for all the models of the scene, one buffer for all the models
        // std::shared_ptr<vireo::Buffer> modelUniformBuffers;
        // Data for all the materials of the scene, one buffer for all the materials
        // std::shared_ptr<vireo::Buffer> materialUniformBuffers;
    };

}