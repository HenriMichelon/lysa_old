/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/Body.h>
module lysa.nodes.collision_object;

import lysa.global;
import lysa.application;
import lysa.physics.jolt.engine;

namespace lysa {


    CollisionObject::CollisionObject(
        const std::shared_ptr<Shape>& shape,
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer},
        shape{shape},
        bodyInterface{dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine()).getBodyInterface()} {
    }

    CollisionObject::CollisionObject(
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer},
        bodyInterface{dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine()).getBodyInterface()} {
    }

    CollisionObject::CollisionObject(const CollisionObject& other):
        Node{other.getName(), other.getType()},
        collisionLayer{other.collisionLayer},
        shape{other.shape},
        bodyInterface{dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine()).getBodyInterface()} {
    }


    void CollisionObject::releaseResources() {
        if (!bodyId.IsInvalid()) {
            if (bodyInterface.IsAdded(bodyId)) {
                bodyInterface.RemoveBody(bodyId);
            }
            bodyInterface.DestroyBody(bodyId);
            bodyId = JPH::BodyID{JPH::BodyID::cInvalidBodyID};
        }
    }

    void CollisionObject::setCollisionLayer(const uint32_t layer) {
        collisionLayer = layer;
        if (!bodyId.IsInvalid()) {
            bodyInterface.SetObjectLayer(bodyId, collisionLayer);
        }
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        if (bodyId.IsInvalid()) { return false; }
        return dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine())
            .getPhysicsSystem()
            .WereBodiesInContact(bodyId, obj->bodyId);
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || bodyId.IsInvalid()|| !bodyInterface.IsAdded(bodyId)) {
            return;
        }
        const auto& position = getPositionGlobal();
        const auto& quat = getRotationGlobal();
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void CollisionObject::setBodyId(const JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()), getName());
    }

    CollisionObject *CollisionObject::getByBodyId(const JPH::BodyID id) {
        return reinterpret_cast<CollisionObject *>(dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine())
            .getBodyInterface()
            .GetUserData(id));
    }

    void CollisionObject::process(const float alpha) {
        Node::process(alpha);
        if (bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) { return; }
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        const auto pos = float3{position.GetX(), position.GetY(), position.GetZ()};
        setPositionGlobal(pos);
        const auto rot = quaternion{rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()};
        setRotationGlobal(rot);
        updating = false;
    }

    void CollisionObject::enterScene() {
        if (isProcessed() && !bodyId.IsInvalid() && isVisible()) {
            if (!bodyInterface.IsAdded(bodyId)) {
                bodyInterface.AddBody(bodyId, activationMode);
            }
            bodyInterface.SetObjectLayer(bodyId, collisionLayer);
            setPositionAndRotation();
        }
        Node::enterScene();
    }

    void CollisionObject::exitScene() {
        if (!bodyId.IsInvalid() && bodyInterface.IsAdded(bodyId)) {
            bodyInterface.RemoveBody(bodyId);
        }
        Node::exitScene();
    }

    void CollisionObject::pause() {
        if (!bodyId.IsInvalid() && bodyInterface.IsAdded(bodyId)) {
            bodyInterface.RemoveBody(bodyId);
        }
        Node::pause();
    }

    void CollisionObject::resume() {
        if (isProcessed() && !bodyId.IsInvalid()) {
            if (isVisible() && (getViewport() != nullptr)) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.AddBody(bodyId, activationMode);
                }
                bodyInterface.SetObjectLayer(bodyId, collisionLayer);
                setPositionAndRotation();
            }
        }
        Node::resume();
    }

    void CollisionObject::setVisible(const bool visible) {
        if (!bodyId.IsInvalid() && visible != this->isVisible()) {
            if (isVisible() && (getViewport() != nullptr)) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.AddBody(bodyId, activationMode);
                }
                bodyInterface.SetObjectLayer(bodyId, collisionLayer);
                setPositionAndRotation();
            } else {
                if (bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.RemoveBody(bodyId);
                }
            }
        }
        Node::setVisible(visible);
    }

}
