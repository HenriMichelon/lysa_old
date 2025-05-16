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
import lysa.nodes.camera;
import lysa.nodes.mesh_instance;
import lysa.nodes.node;
import lysa.nodes.viewport;
import lysa.resources.material;

export namespace lysa {
    struct BufferPair {
        std::shared_ptr<vireo::Buffer> vertexBuffer;
        std::shared_ptr<vireo::Buffer> indexBuffer;
        friend bool operator==(const BufferPair& first, const BufferPair& second) {
            return first.vertexBuffer == second.vertexBuffer && first.indexBuffer == second.indexBuffer;
        }
    };
}

template <>
struct std::hash<lysa::BufferPair> {
    size_t operator()(const lysa::BufferPair& pair) const noexcept {
        const size_t h1 = std::hash<std::shared_ptr<vireo::Buffer>>{}(pair.vertexBuffer);
        const size_t h2 = std::hash<std::shared_ptr<vireo::Buffer>>{}(pair.indexBuffer);
        size_t seed = h1;
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

export namespace lysa {

    class Scene {
    public:
        //! Returns the list of the models of the scene
        const auto& getModels() const { return models; }

        const auto& getOpaqueModels() const { return opaqueModels; }

        const auto& getMaterialIndex(const unique_id materialId) const {
            return materialsIndices.at(materialId);
        }

        const auto& getModelIndex(const unique_id modelId) const {
            return modelsIndices.at(modelId);
        }

        virtual void update() = 0;

        //! Returns the current scene camera
        auto getCurrentCamera() const { return currentCamera; }

        auto isModelsUpdated() const { return modelsUpdated; }

        auto isCameraUpdated() const { return cameraUpdated; }

        auto getViewport() const { return viewport; }

        auto getScissors() const { return scissors; }

        virtual ~Scene() = default;

    protected:
        const RenderingConfig& config;

        // Currently active camera, first camera added to the scene or the last activated
        std::shared_ptr<Camera> currentCamera{};
        // Camera has been updated
        bool cameraUpdated{false};

        // All the models of the scene
        std::list<std::shared_ptr<MeshInstance>> models{};
        // Indices of all models in the buffers
        std::unordered_map<unique_id, uint32> modelsIndices{};
        // All models containing opaque surfaces grouped by buffers
        std::unordered_map<BufferPair, std::list<std::shared_ptr<MeshInstance>>> opaqueModels{};
        // Models have been updated
        bool modelsUpdated{false};

        // All materials used in the scene, used to update the buffer in GPU memory
        std::list<std::shared_ptr<Material>> materials;
        // Material reference counter, used to know of the material, can be removed from the scene
        std::unordered_map<unique_id, uint32> materialsRefCounter;
        // Indices of all materials & texture in the buffers
        std::unordered_map<unique_id, uint32> materialsIndices{};
        // Materials have been updated
        bool materialsUpdated{true};

        Scene(const RenderingConfig& config, const vireo::Extent &extent) : config{config} { resize(extent); }

    private:
        // Rendering window extent
        vireo::Extent extent;
        // Viewport & Scissors
        vireo::Viewport viewport;
        vireo::Rect scissors;
        std::shared_ptr<Viewport> viewportAndScissors{nullptr};

        friend class Window;

        void resize(const vireo::Extent &extent);

        //! Adds a model to the scene
        virtual void addNode(const std::shared_ptr<Node> &node);

        //! Removes a model from the scene
        virtual void removeNode(const std::shared_ptr<Node> &node);

        //! Changes the active camera, disabling the previous camera
        virtual void activateCamera(const std::shared_ptr<Camera> &camera);

        void resetCameraUpdated() { cameraUpdated = false; }

        void resetModelsUpdated() { modelsUpdated = false; }

    };

}
