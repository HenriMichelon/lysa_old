/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
export module lysa.scene;

import vireo;
import lysa.global;
import lysa.nodes.camera;
import lysa.nodes.mesh_instance;
import lysa.nodes.node;
import lysa.nodes.viewport;

export namespace lysa {

    class Scene {
    public:
        //! Returns the list of all the models of the scene
        const auto& getModels() const { return models; }

        virtual void update() = 0;

        //! Returns the current scene camera
        auto getCurrentCamera() const { return currentCamera; }

        auto isModelsUpdated() const { return modelsUpdated; }

        auto isCameraUpdated() const { return cameraUpdated; }

        auto getViewport() const { return viewport; }

        auto getScissors() const { return scissors; }

        virtual ~Scene() = default;

    protected:
        // Currently active camera, first camera added to the scene or the last activated
        std::shared_ptr<Camera> currentCamera{};
        // Camera has been updated
        bool cameraUpdated{false};

        // All the models of the scene
        std::list<std::shared_ptr<MeshInstance>> models{};
        // All models containing opaque surfaces
        //std::map<unique_id, std::list<std::shared_ptr<MeshInstance>>> opaqueModels{};

        // Models have been updated
        bool modelsUpdated{false};

        Scene(const vireo::Extent &extent) { resize(extent); }

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