/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.camera;

import lysa.global;
import lysa.window;

namespace lysa {

    Camera::Camera(const std::wstring &nodeName):
        Node{nodeName, CAMERA} {
    }

    void Camera::setActive(const bool isActive) {
        assert([&]{return  getWindow() != nullptr; }, "Invalid window");
        active = isActive;
        if (active) {
            if (perspectiveProjection) {
                setPerspectiveProjection(fov, nearDistance, farDistance);
            } else {
                setOrthographicProjection(0.0f, 1.0f, 0.0f, 1.0f, nearDistance, farDistance);
            }
            updated = getWindow()->getFramesInFlight();
        }
    }

    void Camera::setNearDistance(const float distance) {
        nearDistance = distance;
        if (perspectiveProjection) setPerspectiveProjection(fov, nearDistance, farDistance);
    }

    void Camera::setFarDistance(const float distance) {
        farDistance = distance;
        if (perspectiveProjection) setPerspectiveProjection(fov, nearDistance, farDistance);
    }

    void Camera::setFov(const float fov) {
        this->fov = fov;
        if (perspectiveProjection) setPerspectiveProjection(fov, nearDistance, farDistance);
    }

    void Camera::setOrthographicProjection(const float left, const float right,
                                           const float top, const float  bottom,
                                           const float near, const float far) {
        perspectiveProjection  = false;
        projectionMatrix       = float4x4::identity();
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }

    void Camera::setPerspectiveProjection(const float fov, const float near, const float far) {
        if (getWindow()) {
            perspectiveProjection  = true;
            this->fov              = fov;
            nearDistance           = near;
            farDistance            = far;
            const auto aspect      = getWindow()->getAspectRatio();
            const float f = 1.0f / std::tan(lysa::radians(lysa::float1{fov}) * 0.5f);
            const float zRange = near - far;
            projectionMatrix = float4x4{
                f / aspect, 0.0f,  0.0f,                         0.0f,
                0.0f,       f,     0.0f,                         0.0f,
                0.0f,       0.0f,  (far + near) / zRange,        -1.0f,
                0.0f,       0.0f,  (2.0f * far * near) / zRange, 0.0f};
        }
    }

    std::shared_ptr<Node> Camera::duplicateInstance() const {
        return std::make_shared<Camera>(*this);
    }

}
