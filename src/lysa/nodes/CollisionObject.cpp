/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.collision_object;

namespace lysa {

    CollisionObject::~CollisionObject() {
        releaseResources();
    }

    void CollisionObject::setProperty(const std::string &property, const std::string &value) {
        Node::setProperty(property, value);
        if (property == "layer") {
            setCollisionLayer(stoul(value));
        }
    }

    void CollisionObject::updateGlobalTransform() {
        Node::updateGlobalTransform();
        setPositionAndRotation();
    }

}
