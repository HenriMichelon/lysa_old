/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.node;
#include <cassert>
import lysa.constants;

namespace lysa {

    id_t Node::currentId = 0;

    Node::Node(const std::wstring &name, const Type type):
        localTransform{glm::mat4{1.0f}},
        id{++currentId},
        type{type},
        name{name} {
        Node::updateGlobalTransform();
    }

    std::wstring Node::sanitizeName(const std::wstring &name) {
        auto newName = name;
        std::ranges::replace(newName, '/', '_');
        std::ranges::replace(newName, ':', '_');
        return newName;
    }

    void Node::ready(const Surface* surface) {
        assert(surface != nullptr);
        this->surface = surface;
        onReady();
    }

    void Node::updateGlobalTransform() {
        if (parent) {
            globalTransform = parent->globalTransform * localTransform;
        } else {
            globalTransform = localTransform;
        }
        for (const auto& child : children) {
            child->updateGlobalTransform();
        }
    }

    glm::vec3 Node::toGlobal(const glm::vec3& local) const {
        return glm::vec3{globalTransform * glm::vec4{local, 1.0f}};
    }

    glm::vec3 Node::toLocal(const glm::vec3& global) const {
        return glm::vec3{inverse(globalTransform) * localTransform * glm::vec4{global, 1.0f}};
    }
    void Node::setPosition(const glm::vec3& position) {
        if (position != getPosition()) {
            localTransform[3] = glm::vec4{position, 1.0f};
            updateGlobalTransform();
        }
    }

    glm::vec3 Node::getRotation() const {
        return glm::eulerAngles(getRotationQuaternion());
    }

    glm::vec3 Node::getRotationGlobal() const {
        return glm::eulerAngles(getRotationQuaternionGlobal());
    }

    void Node::translate(const glm::vec3& localOffset) {
        localTransform = glm::translate(localTransform, localOffset);
        updateGlobalTransform();
    }

    void Node::setPositionGlobal(const glm::vec3& position) {
        if (position != getPositionGlobal()) {
            if (parent == nullptr) {
                setPosition(position);
                return;
            }
            localTransform[3] = inverse(parent->globalTransform) * glm::vec4{position, 1.0};
            updateGlobalTransform();
        }
    }

    void Node::setScale(const glm::vec3& scale) {
        glm::vec3 old_scale, translation, skew;
        glm::vec4 perspective;
        glm::quat orientation;
        decompose(localTransform, old_scale, orientation, translation, skew, perspective);
        if (scale != old_scale) {
            localTransform = glm::translate(translation) *
                                   mat4_cast(orientation) *
                                   glm::scale(scale);
            updateGlobalTransform();
        }
    }

    void Node::setScale(const float scale) {
        setScale(glm::vec3{scale, scale, scale});
    }

    glm::vec3 Node::getScale() const {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        decompose(localTransform, scale, rotation, translation, skew, perspective);
        return scale;
    }

    void Node::rotateTowards(const glm::quat& targetRotation, const float maxAngle) {
        const auto currentRotation = quat_cast(localTransform);
        const auto newRotation = glm::slerp(currentRotation, targetRotation, glm::clamp(maxAngle, 0.0f, 1.0f));
        localTransform = toMat4(newRotation);
        updateGlobalTransform();
    }

    void Node::rotate(const glm::quat& quaternion) {
        localTransform = localTransform * toMat4(quaternion);
        updateGlobalTransform();
    }

    void Node::rotateX(const float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_X);
        updateGlobalTransform();
    }

    void Node::rotateY(const float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_Y);
        updateGlobalTransform();
    }

    void Node::rotateZ(const float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_Z);
        updateGlobalTransform();
    }

    void Node::setRotationX(const float angle) {
        rotateX(angle - getRotationX());
    }

    void Node::setRotationY(const float angle) {
        rotateX(angle - getRotationY());
    }

    void Node::setRotationZ(const float angle) {
        rotateX(angle - getRotationZ());
    }

    void Node::setRotation(const glm::vec3& rot) {
        if (rot != getRotation()) {
            setRotation(glm::quat(rot));
        }
    }

    void Node::setRotation(const glm::quat& quater) {
        if (quater != getRotationQuaternion()) {
            glm::vec3 scale, translation, skew;
            glm::vec4 perspective;
            glm::quat orientation;
            decompose(localTransform, scale, orientation, translation, skew, perspective);
            localTransform = glm::translate(glm::mat4{1.0f}, translation)
                    * toMat4(quater)
                    * glm::scale(glm::mat4{1.0f}, scale);
            updateGlobalTransform();
        }
    }

    glm::vec3 Node::getScaleGlobal() const {
        // Extract scale as the length of the basis vectors (first 3 columns of the 3x3 rotation-scale part)
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(globalTransform[0]));
        scale.y = glm::length(glm::vec3(globalTransform[1]));
        scale.z = glm::length(glm::vec3(globalTransform[2]));
        return scale;
    }

    void Node::setRotationGlobal(const glm::quat& quater) {
        const auto worldRotation = getRotationQuaternionGlobal();
        if (quater != worldRotation) {
            if (parent == nullptr) {
                setRotation(quater);
                return;
            }
            const auto newGlobalTransform = glm::translate(glm::mat4{1.0f}, getPositionGlobal()) *
                                           glm::mat4_cast(quater) *
                                           glm::scale(glm::mat4{1.0f}, getScaleGlobal());
            localTransform = glm::inverse(parent->globalTransform) * newGlobalTransform;
            updateGlobalTransform();
        }
    }

    glm::quat Node::getRotationQuaternion() const {
        return glm::toQuat(glm::mat3(localTransform));
    }

    glm::quat Node::getRotationQuaternionGlobal() const {
        return glm::toQuat(glm::mat3(globalTransform));
    }

}
