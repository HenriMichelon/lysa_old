/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
module lysa.nodes.collision_object;

import lysa.application;
import lysa.global;
import lysa.viewport;
import lysa.physics.jolt.engine;

namespace lysa {

    CollisionObject::CollisionObject(
        const std::shared_ptr<Shape>& shape,
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer},
        shape{shape} {
    }

    CollisionObject::CollisionObject(
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer} {
    }

    CollisionObject::CollisionObject(const CollisionObject& other):
        Node{other.getName(), other.getType()},
        collisionLayer{other.collisionLayer},
        shape{other.shape} {
    }

    JPH::BodyInterface* CollisionObject::getBodyInterface() const {
        if (getViewport()) {
            return &dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene()).getBodyInterface();
        }
        return nullptr;
    }

    void CollisionObject::releaseResources() {
        if (!bodyId.IsInvalid() && bodyInterface) {
            if (bodyInterface->IsAdded(bodyId)) {
                bodyInterface->RemoveBody(bodyId);
            }
            bodyInterface->DestroyBody(bodyId);
            bodyInterface = nullptr;
            bodyId = JPH::BodyID{JPH::BodyID::cInvalidBodyID};
        }
    }

    void CollisionObject::setCollisionLayer(const collision_layer layer) {
        collisionLayer = layer;
        if (!bodyId.IsInvalid() && bodyInterface) {
            bodyInterface->SetObjectLayer(bodyId, collisionLayer);
        }
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        if (bodyId.IsInvalid() || !bodyInterface) { return false; }
        return dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene())
            .getPhysicsSystem()
            .WereBodiesInContact(bodyId, obj->bodyId);
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || bodyId.IsInvalid()|| !bodyInterface || !bodyInterface->IsAdded(bodyId)) {
            return;
        }
        const auto& position = getPositionGlobal();
        const auto& quat = normalize(getRotationGlobal());
        bodyInterface->SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void CollisionObject::scale(const float scale) {
        Node::scale(scale);
        if (bodyId.IsInvalid()|| !bodyInterface || !bodyInterface->IsAdded(bodyId)) {
            return;
        }
        if (scale != 1.0f) {
            bodyInterface->SetShape(
                bodyId,
                new JPH::ScaledShape(
                    bodyInterface->GetShape(bodyId),
                    JPH::Vec3{scale, scale, scale}),
                false,
                activationMode);
        }
    }

    void CollisionObject::setBodyId(const JPH::BodyID id) {
        bodyId = id;
        bodyInterface->SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()), getName());
    }

    void CollisionObject::process(const float alpha) {
        Node::process(alpha);
        if (bodyId.IsInvalid() || !bodyInterface || !bodyInterface->IsAdded(bodyId)) { return; }
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface->GetPositionAndRotation(bodyId, position, rotation);
        const auto pos = float3{position.GetX(), position.GetY(), position.GetZ()};
        if (any(pos != getPositionGlobal())) {
            setPositionGlobal(pos);
        }
        const auto rot = quaternion{rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()};
        if (any(rot != getRotationGlobal())) {
            setRotationGlobal(rot);
        }
        updating = false;
    }

    void CollisionObject::attachToViewport(Viewport* viewport) {
        Node::attachToViewport(viewport);
        bodyInterface = getBodyInterface();
    }

    void CollisionObject::enterScene() {
        bodyInterface->SetObjectLayer(bodyId, collisionLayer);
        if (isProcessed() && isVisible()) {
            bodyInterface->AddBody(bodyId, activationMode);
            setPositionAndRotation();
        }
        Node::enterScene();
    }

    void CollisionObject::exitScene() {
        if (!bodyId.IsInvalid() && bodyInterface) {
            releaseResources();
        }
        Node::exitScene();
    }

    void CollisionObject::pause() {
        if (!bodyId.IsInvalid() && bodyInterface && bodyInterface->IsAdded(bodyId)) {
            bodyInterface->RemoveBody(bodyId);
        }
        Node::pause();
    }

    void CollisionObject::resume() {
        if (isProcessed() && !bodyId.IsInvalid()) {
            if (isVisible() && bodyInterface) {
                if (!bodyInterface->IsAdded(bodyId)) {
                    bodyInterface->AddBody(bodyId, activationMode);
                }
                bodyInterface->SetObjectLayer(bodyId, collisionLayer);
                setPositionAndRotation();
            }
        }
        Node::resume();
    }

    void CollisionObject::setVisible(const bool visible) {
        if (!bodyId.IsInvalid() && visible != this->isVisible() && bodyInterface) {
            if (isVisible()) {
                if (!bodyInterface->IsAdded(bodyId)) {
                    bodyInterface->AddBody(bodyId, activationMode);
                }
                bodyInterface->SetObjectLayer(bodyId, collisionLayer);
                setPositionAndRotation();
            } else {
                if (bodyInterface->IsAdded(bodyId)) {
                    bodyInterface->RemoveBody(bodyId);
                }
            }
        }
        Node::setVisible(visible);
    }

}
