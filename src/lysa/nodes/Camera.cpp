/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.camera;

import lysa.global;
import lysa.viewport;

namespace lysa {

    Camera::Camera(const std::wstring &nodeName):
        Node{nodeName, CAMERA} {
    }

    void Camera::setActive(const bool isActive) {
        assert([&]{return  getViewport() != nullptr; }, "Invalid window");
        active = isActive;
        if (active) {
            if (perspectiveProjection) {
                setPerspectiveProjection(fov, nearDistance, farDistance);
            } else {
                setOrthographicProjection(left, right, top, bottom, nearDistance, farDistance);
            }
            setUpdated();
        }
    }

    void Camera::setNearDistance(const float distance) {
        nearDistance = distance;
        if (perspectiveProjection) {
            setPerspectiveProjection(fov, nearDistance, farDistance);
        } else {
            setOrthographicProjection(left, right, top, bottom, nearDistance, farDistance);
        }
    }

    void Camera::setFarDistance(const float distance) {
        farDistance = distance;
        if (perspectiveProjection) {
            setPerspectiveProjection(fov, nearDistance, farDistance);
        }
    }

    void Camera::setFov(const float fov) {
        this->fov = fov;
        if (perspectiveProjection) {
            setPerspectiveProjection(fov, nearDistance, farDistance);
        } else {
            setOrthographicProjection(left, right, top, bottom, nearDistance, farDistance);
        }
    }

    void Camera::setOrthographicProjection(const float left, const float right,
                                           const float top, const float  bottom,
                                           const float near, const float far) {
        this->left = left;
        this->right = right;
        this->top = top;
        this->bottom = bottom;
        nearDistance           = near;
        farDistance            = far;
        perspectiveProjection  = false;
        projectionMatrix       = orthographic(left, right, top, bottom, near, far);
        setUpdated();
    }

    void Camera::setPerspectiveProjection(const float fov, const float near, const float far) {
        if (getViewport()) {
            perspectiveProjection  = true;
            this->fov              = fov;
            nearDistance           = near;
            farDistance            = far;
            projectionMatrix = perspective(
                radians(fov),
                getViewport()->getAspectRatio(),
                near,
                far);
            setUpdated();
        }
    }

    std::shared_ptr<Node> Camera::duplicateInstance() const {
        return std::make_shared<Camera>(*this);
    }

}
