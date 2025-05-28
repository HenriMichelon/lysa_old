/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.node;

import lysa.global;
import lysa.viewport;

namespace lysa {

    unique_id Node::currentId{INVALID_ID};

    Node::Node(const Node &node):
        id{currentId++} {
        name            = node.name;
        localTransform  = node.localTransform;
        globalTransform = node.globalTransform;
        processMode     = node.processMode;
        type            = node.type;
    }

    Node::Node(const std::wstring &name, const Type type):
        localTransform{float4x4::identity()},
        id{++currentId},
        type{type},
        name{sanitizeName(name)} {
        Node::updateGlobalTransform();
    }

    void Node::ready(Viewport* viewport) {
        assert([&]{return viewport != nullptr; }, "Invalid viewport");
        this->viewport = viewport;
        for (const auto& child : children) {
            child->ready(viewport);
        }
        if (isProcessed()) {
            onReady();
        }
    }

    void Node::physicsProcess(const float delta) {
        for (const auto& child : children) {
            child->physicsProcess(delta);
        }
        if (isProcessed()) {
            onPhysicsProcess(delta);
        }
    }

    void Node::process(const float alpha) {
        for (const auto& child : children) {
            child->process(alpha);
        }
        if (isProcessed()) {
            onProcess(alpha);
        }
    }

    void Node::updateGlobalTransform() {
        if (parent) {
            globalTransform = mul(parent->globalTransform, localTransform);
        } else {
            globalTransform = localTransform;
        }
        setUpdated();
        for (const auto& child : children) {
            child->updateGlobalTransform();
        }
    }

    std::shared_ptr<Node> Node::duplicateInstance() const {
        return std::make_shared<Node>(*this);
    }

    void Node::setPosition(const float3& position) {
        if (any(position != getPosition())) {
            localTransform[3] = float4{position, 1.0f};
            updateGlobalTransform();
        }
    }

    void Node::rotateY(const float angle) {
        localTransform = mul(float4x4::rotation_y(angle), localTransform);
        updateGlobalTransform();
    }

    void Node::rotateGlobalY(float angle) {
        localTransform = mul(localTransform, float4x4::rotation_y(angle));
        updateGlobalTransform();
    }


    bool Node::addChild(const std::shared_ptr<Node> child, const bool async) {
        if (haveChild(child, false)) { return false; }
        assert([&]{return child->parent == nullptr; }, "Remove child from parent first");
        child->parent = this;
        children.push_back(child);
        child->updateGlobalTransform();
        if (viewport) {
            viewport->addNode(child, false);
            child->ready(viewport);
        }
        // child->visible = visible && child->visible;
        // child->castShadows = castShadows;
        // child->dontDrawEdges = dontDrawEdges;
        // if (addedToScene) { app()._addNode(child, async); }
        return true;
    }

    bool Node::removeChild(const std::shared_ptr<Node>& child, const bool async) {
        if (!haveChild(child, false)) { return false; }
        child->parent = nullptr;
        // if (node->addedToScene) { app()._removeNode(node, async); }
        children.remove(child);
        return true;
    }

    bool Node::haveChild(const std::shared_ptr<Node>& child, const bool recursive) const {
        if (!child) { return false;}
        if (recursive) {
            if (haveChild(child, false)) {
                return true;
            }
            for (const auto &node : children) {
                if (node->haveChild(child, true))
                    return true;
            }
            return false;
        }
        return std::ranges::find(children, child) != children.end();
    }

    bool Node::isProcessed() const {
        const auto paused = viewport == nullptr || viewport->isPaused();
        auto       mode   = processMode;
        if ((parent == nullptr) && (mode == ProcessMode::INHERIT))
            mode = ProcessMode::PAUSABLE;
        return ((mode == ProcessMode::INHERIT) && (parent->isProcessed())) ||
                (!paused && (mode == ProcessMode::PAUSABLE)) ||
                (paused && (mode == ProcessMode::WHEN_PAUSED)) ||
                (mode == ProcessMode::ALWAYS);
    }


    /*
    float3 Node::toGlobal(const float3& local) const {
        return mul(globalTransform, float4{local, 1.0f}).xyz;
    }

    float3 Node::toLocal(const float3& global) const {
        return mul(mul(inverse(globalTransform), localTransform), float4{global, 1.0f}).xyz;
    }

    void Node::setPositionGlobal(const float3& position) {
        if (any(position != getPositionGlobal())) {
            if (parent == nullptr) {
                setPosition(position);
                return;
            }
            localTransform[3] = mul(inverse(parent->globalTransform), float4{position, 1.0});
            updateGlobalTransform();
        }
    }

    float3 Node::getRotation() const {
    }
    
    float3 Node::getRotationGlobal() const {
    }
    
    void Node::translate(const float3& localOffset) {
        localTransform = mul(localTransform, float4x4::translation(localOffset));
        updateGlobalTransform();
    }
    
    void Node::setScale(const float3& scale) {
        if (any(scale != getScale())) {
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
        if (any(rot != getRotation())) {
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
