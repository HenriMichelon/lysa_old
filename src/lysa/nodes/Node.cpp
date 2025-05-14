/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.node;
#include <cassert>

import lysa.global;

namespace lysa {

    id_t Node::currentId{INVALID_ID};

    Node::Node(const std::wstring &name, const Type type):
        // localTransform{float4x4{1.0f}},
        id{++currentId},
        type{type},
        name{sanitizeName(name)} {
        // Node::updateGlobalTransform();
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

    // float3 Node::toGlobal(const float3& local) const {
    //     return mul(globalTransform, float4{local, 1.0f}).xyz;
    // }

    // float3 Node::toLocal(const float3& global) const {
    //     return mul(mul(inverse(globalTransform), localTransform), float4{global, 1.0f}).xyz;
    // }

    // void Node::setPosition(const float3& position) {
    //     if (all(position == getPosition())) {
    //         localTransform[3] = float4{position, 1.0f};
    //         updateGlobalTransform();
    //     }
    // }

    // void Node::setPositionGlobal(const float3& position) {
    //     if (all(position != getPositionGlobal())) {
    //         if (parent == nullptr) {
    //             setPosition(position);
    //             return;
    //         }
    //         localTransform[3] = mul(inverse(parent->globalTransform), float4{position, 1.0});
    //         updateGlobalTransform();
    //     }
    // }

/*    float3 Node::getRotation() const {
    }
    
    float3 Node::getRotationGlobal() const {
    }
    
    void Node::translate(const float3& localOffset) {
        localTransform = mul(localTransform, float4x4::translation(localOffset));
        updateGlobalTransform();
    }
    
    void Node::setScale(const float3& scale) {
        if (all(scale != getScale())) {
            float3 x = normalize(localTransform[0].xyz);
            float3 y = normalize(localTransform[1].xyz);
            float3 z = normalize(localTransform[2].xyz);
            localTransform[0].xyz = x * scale.x;
            localTransform[1].xyz = y * scale.y;
            localTransform[2].xyz = z * scale.z;
            updateGlobalTransform();
        }
    }
    
    void Node::setScale(const float scale) {
        setScale(float3{scale, scale, scale});
    }
    
    float3 Node::getScale() const {
        float3 scale;
        scale.x = length(localTransform[0].xyz);
        scale.y = length(localTransform[1].xyz);
        scale.z = length(localTransform[2].xyz);
        return scale;
    }
    
    void Node::rotateTowards(const quaternion& targetRotation, const float maxAngle) {
        updateGlobalTransform();
    }
    
    void Node::rotate(const quaternion& quaternion) {
        updateGlobalTransform();
    }
    
    void Node::rotateX(const float angle) {
        updateGlobalTransform();
    }
    
    void Node::rotateY(const float angle) {
        updateGlobalTransform();
    }
    
    void Node::rotateZ(const float angle) {
        updateGlobalTransform();
    }
    
    void Node::setRotationX(const float angle) {
    }
    
    void Node::setRotationY(const float angle) {
    }
    
    void Node::setRotationZ(const float angle) {
    }
    
    void Node::setRotation(const float3& rot) {
        if (all(rot != getRotation())) {
            setRotation(quaternion(rot));
        }
    }

    void Node::setRotation(const quaternion& quat) {

    }

    float3 Node::getScaleGlobal() const {
        float3 scale;
        scale.x = length(globalTransform[0].xyz);
        scale.y = length(globalTransform[1].xyz);
        scale.z = length(globalTransform[2].xyz);
        return scale;
    }

    void Node::setRotationGlobal(const quaternion& quat) {

    }

    quaternion Node::getRotationQuaternion() const {
    }

    quaternion Node::getRotationQuaternionGlobal() const {
    }*/
    
}
