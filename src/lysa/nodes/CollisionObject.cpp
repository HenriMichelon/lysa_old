/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.collision_object;

namespace lysa {

    CollisionObject::CollisionObject(
        const std::shared_ptr<Shape>& shape,
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer},
        shape{shape},
        physX{dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine())} {
    }

    CollisionObject::CollisionObject(
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer},
        physX{dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine())} {
    }

    CollisionObject::CollisionObject(const CollisionObject& other):
        Node{other.getName(), other.getType()},
        collisionLayer{other.collisionLayer},
        shape{other.shape},
        physX{dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine())} {
    }

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
