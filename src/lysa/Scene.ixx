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
        const auto& getOpaqueModels() const { return opaqueModels; }

        const auto& getOpaqueDrawCommands() const { return opaqueDrawCommands; }

        const auto& getOpaqueDrawCommandsBuffer() const { return opaqueDrawCommandsBuffer; }

        const auto& getMaterialIndex(const unique_id materialId) const {
            return materialsIndices.at(materialId);
        }

        virtual void update() = 0;

        //! Returns the current scene camera
        auto getCurrentCamera() const { return currentCamera; }

        auto isCameraUpdated() const { return cameraUpdated; }

        auto getViewport() const { return viewport; }

        auto getScissors() const { return scissors; }

        virtual ~Scene() = default;

    protected:
        const RenderingConfiguration& config;

        // Currently active camera, first camera added to the scene or the last activated
        std::shared_ptr<Camera> currentCamera{};
        // Camera has been updated
        bool cameraUpdated{false};

        // All models containing opaque surfaces grouped by buffers
        std::unordered_map<BufferPair, std::list<std::shared_ptr<MeshInstance>>> opaqueModels{};
        std::unordered_map<BufferPair, std::vector<vireo::DrawIndexedIndirectCommand>> opaqueDrawCommands{};
        std::unordered_map<BufferPair, std::shared_ptr<vireo::Buffer>> opaqueDrawCommandsBuffer;

        // All materials used in the scene, used to update the buffer in GPU memory
        std::vector<std::shared_ptr<Material>> materials;
        // Index used for faster updates
        uint32 lastMaterialIndex{0};
        // Material reference counter, used to know if the material is still used or if it can be removed from the scene
        std::unordered_map<unique_id, uint32> materialsRefCounter;
        // Indices of all materials in the buffers
        std::unordered_map<unique_id, uint32> materialsIndices{};

        Scene(const RenderingConfiguration& config, const std::shared_ptr<vireo::Vireo>& vireo, const vireo::Extent &extent);

    private:
        // Rendering window extent
        vireo::Extent extent;
        // Viewport & Scissors
        vireo::Viewport viewport;
        vireo::Rect scissors;
        std::shared_ptr<Viewport> viewportAndScissors{nullptr};
        const std::shared_ptr<vireo::Vireo>& vireo;
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        std::shared_ptr<vireo::CommandAllocator> commandAllocator;

        friend class Window;

        bool updateMaterial(const std::shared_ptr<Material>& material);

        void resize(const vireo::Extent &extent);

        //! Adds a model to the scene
        virtual void addNode(const std::shared_ptr<Node> &node);

        //! Removes a model from the scene
        virtual void removeNode(const std::shared_ptr<Node> &node);

        //! Changes the active camera, disabling the previous camera
        virtual void activateCamera(const std::shared_ptr<Camera> &camera);

    };

}
