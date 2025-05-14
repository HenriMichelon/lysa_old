/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
export module lysa.renderers.scene_data;

import vireo;
import lysa.global;
import lysa.nodes.camera;
import lysa.nodes.mesh_instance;

export namespace lysa {

    struct SceneUniform {
        float3      cameraPosition;
        alignas(16) float4x4 projection;
        float4x4    view;
        float4x4    viewInverse;
        float4      ambientLight{1.0f, 1.0f, 1.0f, 0.01f}; // RGB + strength
    };

    struct ModelUniform {
        float4x4 transform;
    };

    struct MaterialUniform {
        alignas(16) float shininess{128.f};
        alignas(4)  int32 diffuseTextureIndex{-1};
    };

    struct SceneData {
        // Currently active camera, first camera added to the scene or the last activated
        std::shared_ptr<Camera> currentCamera{};
        // All models containing opaque surfaces
        std::map<unique_id, std::list<std::shared_ptr<MeshInstance>>> opaquesModels{};
        // Data for all the models of the scene, one array and one buffer for all the models
        std::unique_ptr<ModelUniform[]> modelUniforms;
        std::unique_ptr<vireo::Buffer>  modelUniformBuffer;

    };

}