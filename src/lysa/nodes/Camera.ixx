/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.camera;

import lysa.global;
import lysa.nodes.node;

export namespace lysa {

    /**
     * Camera node, displays from a point of view.
     */
    class Camera : public Node {
    public:
        /**
         * Creates a Camera
         */
        Camera(const std::string &nodeName = TypeNames[CAMERA]);

        /**
         * Returns `true` if the camera is the currently active camera for the current scene.
         * Use Application::activateCamera() to activate a camera
         */
        auto isActive() const { return active; }

        /**
         * Sets the camera projection to orthogonal mode.
         * @param left, right, top, bottom size of the view
         * @param near, far clip planes
         */
        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

        /**
         * Sets the camera projection to perspective mode.
         * @param fov field of view angle in degrees
         * @param near nearest clip plane
         * @param far farthest clip plane
         */
        void setPerspectiveProjection(float fov, float near, float far);

        /**
         * Returns the projection matrix
         */
        const auto& getProjection() const { return projectionMatrix; }

        /**
         * Returns the camera near clipping distance
         */
        auto getNearDistance() const { return nearDistance; }

        /**
         * Sets the near clipping distance
         */
        void setNearDistance(float distance);

        /**
         * Returns the camera far clipping distance
         */
        auto getFarDistance() const { return farDistance; }

        /**
         * Sets the far clipping distance
         */
        void setFarDistance(float distance);

        /**
         * Returns the camera FOV in degrees
         */
        auto getFov() const { return fov; }

        /**
         * Sets the camera FOV in degrees
        */
        void setFov(float fov);


    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

    private:
        friend class Scene;

        // Field of view in degrees
        float fov{75.0};
        // Nearest clipping distance
        float nearDistance{0.05f};
        // Furthest clipping distance
        float farDistance{100.0f};
        // Is this the currently active camera?
        bool active{false};
        // Is the projection perspective?
        bool perspectiveProjection{true};
        // Projection matrix for the global
        float4x4 projectionMatrix{1.0f};
        // Parameters for orthographic projection:
        float left;
        float right;
        float top;
        float bottom;

        void setActive(bool isActive);
    };

}
