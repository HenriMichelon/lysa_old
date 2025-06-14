/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.collision_object;

import lysa.global;
import lysa.viewport;
import lysa.physics.physx.engine;

namespace lysa {

    void CollisionObject::releaseResources() {
        // if (!bodyId.IsInvalid() && bodyInterface) {
        //     if (bodyInterface->IsAdded(bodyId)) {
        //         bodyInterface->RemoveBody(bodyId);
        //     }
        //     bodyInterface->DestroyBody(bodyId);
        //     bodyInterface = nullptr;
        //     bodyId = JPH::BodyID{JPH::BodyID::cInvalidBodyID};
        // }
    }

    void CollisionObject::setCollisionLayer(const collision_layer layer) {
        collisionLayer = layer;
        // if (!bodyId.IsInvalid() && bodyInterface) {
        //     bodyInterface->SetObjectLayer(bodyId, collisionLayer);
        // }
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        // if (bodyId.IsInvalid() || !bodyInterface) { return false; }
        // return dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene())
        //     .getPhysicsSystem()
        //     .WereBodiesInContact(bodyId, obj->bodyId);
        return false;
    }

    void CollisionObject::setPositionAndRotation() {
        // if (updating || bodyId.IsInvalid()|| !bodyInterface || !bodyInterface->IsAdded(bodyId)) {
        //     return;
        // }
        // const auto& position = getPositionGlobal();
        // const auto& quat = normalize(getRotationGlobal());
        // bodyInterface->SetPositionAndRotation(
        //         bodyId,
        //         JPH::RVec3(position.x, position.y, position.z),
        //         JPH::Quat(quat.x, quat.y, quat.z, quat.w),
        //         activationMode);
    }

    void CollisionObject::process(const float alpha) {
        Node::process(alpha);
        // if (bodyId.IsInvalid() || !bodyInterface || !bodyInterface->IsAdded(bodyId)) { return; }
        updating = true;
        // JPH::Vec3 position;
        // JPH::Quat rotation;
        // bodyInterface->GetPositionAndRotation(bodyId, position, rotation);
        // const auto pos = float3{position.GetX(), position.GetY(), position.GetZ()};
        // setPositionGlobal(pos);
        // const auto rot = quaternion{rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()};
        // setRotationGlobal(rot);
        updating = false;
    }

    void CollisionObject::attachToViewport(Viewport* viewport) {
        Node::attachToViewport(viewport);
        // bodyInterface = getBodyInterface();
    }

    void CollisionObject::enterScene() {
        // bodyInterface->SetObjectLayer(bodyId, collisionLayer);
        // if (isProcessed() && isVisible()) {
        //     bodyInterface->AddBody(bodyId, activationMode);
        //     setPositionAndRotation();
        // }
        Node::enterScene();
    }

    void CollisionObject::exitScene() {
        // if (!bodyId.IsInvalid() && bodyInterface) {
        //     releaseResources();
        // }
        Node::exitScene();
    }

    void CollisionObject::pause() {
        // if (!bodyId.IsInvalid() && bodyInterface && bodyInterface->IsAdded(bodyId)) {
        //     bodyInterface->RemoveBody(bodyId);
        // }
        Node::pause();
    }

    void CollisionObject::resume() {
        // if (isProcessed() && !bodyId.IsInvalid()) {
        //     if (isVisible() && bodyInterface) {
        //         if (!bodyInterface->IsAdded(bodyId)) {
        //             bodyInterface->AddBody(bodyId, activationMode);
        //         }
        //         bodyInterface->SetObjectLayer(bodyId, collisionLayer);
        //         setPositionAndRotation();
        //     }
        // }
        Node::resume();
    }

    void CollisionObject::setVisible(const bool visible) {
        // if (!bodyId.IsInvalid() && visible != this->isVisible() && bodyInterface) {
        //     if (isVisible()) {
        //         if (!bodyInterface->IsAdded(bodyId)) {
        //             bodyInterface->AddBody(bodyId, activationMode);
        //         }
        //         bodyInterface->SetObjectLayer(bodyId, collisionLayer);
        //         setPositionAndRotation();
        //     } else {
        //         if (bodyInterface->IsAdded(bodyId)) {
        //             bodyInterface->RemoveBody(bodyId);
        //         }
        //     }
        // }
        Node::setVisible(visible);
    }

}
