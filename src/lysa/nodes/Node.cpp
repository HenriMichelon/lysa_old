/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.node;

import lysa.global;
import lysa.log;
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

    void Node::ready() {
        assert([&]{return viewport != nullptr; }, "Invalid viewport");
        for (const auto& child : children) {
            child->ready();
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

    void Node::setTransformLocal(const float4x4 &transform) {
        localTransform = transform;
        updateGlobalTransform();
    }

    void Node::updateGlobalTransform() {
        const auto parentMatrix = parent == nullptr ? float4x4::identity() : parent->globalTransform;
        globalTransform = mul(localTransform, parentMatrix);
        setUpdated();
        for (const auto& child : children) {
            child->updateGlobalTransform();
        }
    }

    void Node::exitScene() {
        onExitScene();
        viewport = nullptr;
        for (const auto& child : children) {
            child->exitScene();
        }
    }

    void Node::attachToViewport(Viewport* viewport) {
        this->viewport = viewport;
        for (const auto& child : children) {
            child->attachToViewport(viewport);
        }
    }

    void Node::detachFromViewport() {
        this->viewport = nullptr;
        for (const auto& child : children) {
            child->detachFromViewport();
        }
    }

    void Node::enterScene() {
        this->viewport = viewport;
        for (const auto& child : children) {
            child->enterScene();
        }
        onEnterScene();
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

    void Node::setPositionGlobal(const float3& position) {
        if (any(position != getPositionGlobal())) {
            if (parent == nullptr) {
                setPosition(position);
                return;
            }
            localTransform[3] = mul(float4{position, 1.0}, inverse(parent->globalTransform));
            updateGlobalTransform();
        }
    }

    float3 Node::getRotationEulerAngles() const {
        return eulerAngles(getRotation());
    }

    float3 Node::getRotationEulerAnglesGlobal() const {
        return eulerAngles(getRotation());
    }

    void Node::translate(const float3& localOffset) {
        localTransform = mul(localTransform, float4x4::translation(localOffset));
        updateGlobalTransform();
    }

    void Node::translate(const float x, const float y, const float z) {
        translate(float3{x, y, z});
    }

    void Node::scale(const float scale) {
        localTransform = mul(float4x4::scale(scale), localTransform);
        updateGlobalTransform();
    }

    void Node::setVisible(const bool visible) {
        lockViewportUpdates();
        this->visible = visible;
        for (const auto &child : children) {
            child->setVisible(visible);
        }
        setUpdated();
        unlockViewportUpdates();
    }

    bool Node::addChild(const std::shared_ptr<Node> child, const bool async) {
        if (haveChild(child, false)) { return false; }
        if (child->parent) {
            child->parent->removeChild(child, async);
        }
        child->parent = this;
        children.push_back(child);
        child->updateGlobalTransform();
        if (viewport) {
            viewport->addNode(child, async);
            child->ready();
        }
        child->visible = visible && child->visible;
        return true;
    }

    bool Node::removeChild(const std::shared_ptr<Node>& child, const bool async) {
        if (!haveChild(child, false)) { return false; }
        child->parent = nullptr;
        if (child->viewport) {
            child->viewport->removeNode(child, async);
        }
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
        auto mode = processMode;
        if ((parent == nullptr) && (mode == ProcessMode::INHERIT)) {
            mode = ProcessMode::PAUSABLE;
        }
        return ((mode == ProcessMode::INHERIT) && (parent->isProcessed())) ||
                (!paused && (mode == ProcessMode::PAUSABLE)) ||
                (paused && (mode == ProcessMode::WHEN_PAUSED)) ||
                (mode == ProcessMode::ALWAYS);
    }

    float3 Node::getScale() const {
        return {
            length(localTransform[0].xyz),
            length(localTransform[1].xyz),
            length(localTransform[2].xyz),
        };
    }

    float3 Node::getScaleGlobal() const {
        return {
            length(globalTransform[0].xyz),
            length(globalTransform[1].xyz),
            length(globalTransform[2].xyz),
        };
    }

    /*

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

    void Node::rotateTowards(const quaternion& targetRotation, const float maxAngle) {
        updateGlobalTransform();
    }

    float3 Node::getScaleGlobal() const {
        float3 scale;
        scale.x = length(globalTransform[0].xyz);
        scale.y = length(globalTransform[1].xyz);
        scale.z = length(globalTransform[2].xyz);
        return scale;
    }
*/

    float Node::getRotationX() const {
        const auto angles = eulerAngles(getRotation());
        return angles.x;
    }

    float Node::getRotationY() const {
        const auto angles = eulerAngles(getRotation());
        return angles.y;
    }

    float Node::getRotationZ() const {
        const auto angles = eulerAngles(getRotation());
        return angles.z;
    }

    void Node::setRotationX(const float angle) {
        rotateX(angle - getRotationX());
    }

    void Node::setRotationY(const float angle) {
        rotateY(angle - getRotationY());
    }

    void Node::setRotationZ(const float angle) {
        rotateZ(angle - getRotationZ());
    }

    void Node::setRotation(const quaternion& quat) {
        const auto tm = float4x4::translation(getPosition());
        const auto rm = float4x4{quat};
        const auto sm = float4x4::scale(getScale());
        localTransform = mul(mul(rm, sm), tm);
        updateGlobalTransform();
    }

    void Node::rotateX(const float angle) {
        localTransform = mul(float4x4::rotation_x(angle), localTransform);
        updateGlobalTransform();
    }

    void Node::rotateY(const float angle) {
        localTransform = mul(float4x4::rotation_y(angle), localTransform);
        updateGlobalTransform();
    }

    void Node::rotateZ(const float angle) {
        localTransform = mul(float4x4::rotation_z(angle), localTransform);
        updateGlobalTransform();
    }

    void Node::setRotationGlobal(const quaternion& quat) {
        if (!parent) {
            setRotation(quat);
            return;
        }
        const auto tm = float4x4::translation(getPositionGlobal());
        const auto rm = float4x4{quat};
        const auto sm = float4x4::scale(getScaleGlobal());
        const auto newGlobalTransform = mul(mul(rm, sm), tm);
        localTransform = mul(newGlobalTransform, inverse(parent->globalTransform));
        updateGlobalTransform();
    }

    quaternion Node::getRotation() const {
        return quaternion{float3x3{localTransform}};
    }

    quaternion Node::getRotationGlobal() const {
        return quaternion{float3x3{globalTransform}};
    }

    float3 Node::toGlobal(const float3& local) const {
        return mul(float4(local, 1.0f), globalTransform).xyz;
    }

    float3 Node::toLocal(const float3& global) const {
        return mul(float4(global, 1.0f), mul(localTransform, inverse(globalTransform))).xyz;
    }

    std::wstring Node::getPath() const {
        if (parent) {
            return parent->getPath() + L"/" + getName();
        }
        return  L"/" + name;
    }

    void Node::setProperty(const std::string &property, const std::string &value) {
        if (property == "position") {
            setPosition(to_float3(value));
        } else if (property == "rotation") {
            setRotation(quaternion{to_float3(value)});
        } else if (property == "scale") {
            // setScale(to_float3(value));
        } else if (property == "groups") {
            for(const auto groupName : split(value, ';')) {
                addToGroup(to_wstring(groupName.data()));
            }
        } else if (property == "process_mode") {
            const auto v = to_lower(value);
            if (v == "inherit") {
                setProcessMode(ProcessMode::INHERIT);
            } else if (v == "pausable") {
                setProcessMode(ProcessMode::PAUSABLE);
            } else if (v == "when_paused") {
                setProcessMode(ProcessMode::WHEN_PAUSED);
            } else if (v == "always") {
                setProcessMode(ProcessMode::ALWAYS);
            } else if (v == "disabled") {
                setProcessMode(ProcessMode::DISABLED);
            }
        } else if (property == "visible") {
            setVisible(value == "true");
        } else if (property == "name") {
            setName(std::to_wstring(value));
        } else  if (property == "cast_shadows") {
            // setCastShadows(value == "true");
        }
    }

    std::shared_ptr<Node> Node::duplicate() const {
        std::shared_ptr<Node> dup = duplicateInstance();
        dup->children.clear();
        for (const auto &child : children) {
            dup->addChild(child->duplicate());
        }
        dup->id   = currentId++;
        dup->name = name;
        return dup;
    }

    void Node::printTree(const int tab) const {
        std::stringstream sstream;
        for (int i = 0; i < (tab * 2); i++) {
            sstream << " ";
        }
        sstream << " " << lysa::to_string(name) << " (" << to_string(TypeNames[type]) << ") #" << getId();
        const auto& pos = getPosition();
        sstream << " (" << pos.x << ", " << pos.y << ", " << pos.z << ")";
        INFO(sstream.str());
        for (auto &child : children) {
            child->printTree(tab + 1);
        }
    }

    void Node::killTween(const std::shared_ptr<Tween> &tween) {
        if (tween != nullptr) {
            tween->kill();
            tweens.remove(tween);
        }
    }

    void Node::lockViewportUpdates() {
        if (viewport) {
            viewport->lockDeferredUpdate();
        }
    }

    void Node::unlockViewportUpdates() {
        if (viewport) {
            viewport->unlockDeferredUpdate();
        }
    }

}
