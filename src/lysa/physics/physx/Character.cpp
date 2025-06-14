/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.character;

import lysa.application;
import lysa.global;
import lysa.viewport;
import lysa.nodes.node;
import lysa.physics.physx.engine;

namespace lysa {

    void Character::setShape(const float height, const float radius) {
        assert([&]{ return height/2 > radius; }, "Invalid capsule shape: height/2 < radius");
        const auto position= getPositionGlobal();
        const auto quat = normalize(getRotationGlobal());
        // const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        // const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        // auto& physicsScene = dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene());

        setCollisionLayer(collisionLayer);
    }

    float3 Character::getGroundVelocity() const {
        // const auto velocity = virtualCharacter->GetGroundVelocity();
        // return float3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
        return FLOAT3ZERO;
    }

    Node* Character::getGround() const {
        // return reinterpret_cast<Node*>(virtualCharacter->GetGroundUserData());
        return nullptr;
    }

    bool Character::isOnGround() const {
        return false;
        // return virtualCharacter->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
    }

    bool Character::isGround(const CollisionObject &object) const {
        return false;
        // return object.getBodyId() == virtualCharacter->GetGroundBodyID();
    }

    void Character::setUp(const float3& vector) {
        upVector = vector;
        // virtualCharacter->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
    }

    std::list<CollisionObject::Collision> Character::getCollisions() const {
        std::list<Collision> contacts;
        // for (const auto &contact : virtualCharacter->GetActiveContacts()) {
        //     auto *node = reinterpret_cast<CollisionObject *>(bodyInterface->GetUserData(contact.mBodyB));
        //     assert([&] { return node; }, "physics body not associated with a node");
        //     contacts.push_back({
        //         .position = float3{contact.mPosition.GetX(), contact.mPosition.GetY(), contact.mPosition.GetZ()},
        //         .normal = float3{contact.mSurfaceNormal.GetX(),
        //                        contact.mSurfaceNormal.GetY(),
        //                        contact.mSurfaceNormal.GetZ()},
        //         .object = node
        //     });
        // }
        return contacts;
    }

    void Character::setVelocity(const float3& velocity) const {
        if (all(velocity == FLOAT3ZERO)) {
            // virtualCharacter->SetLinearVelocity(JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            // const auto vel = mul(velocity, getRotationGlobal());
            // virtualCharacter->SetLinearVelocity(JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    void Character::setMaxSlopeAngle(const float angle) const {
        // virtualCharacter->SetMaxSlopeAngle(radians(angle));
    }

    void Character::setPositionAndRotation() {
        // if (updating || !bodyInterface) {
        //     return;
        // }
        // const auto position = getPositionGlobal();
        // const auto quat = normalize(getRotationGlobal());
        // const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        // const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        // virtualCharacter->SetPosition(pos);
        // virtualCharacter->SetRotation(rot);
    }

    void Character::physicsProcess(const float delta) {
        Node::physicsProcess(delta);
        // auto& physicsScene = dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene());
        // virtualCharacter->Update(delta,
        //       virtualCharacter->GetUp() * physicsScene.getPhysicsSystem().GetGravity().Length(),
        //       *this,
        //       *objectLayerFilter,
        //       *this,
        //       {},
        //       physicsScene.getTempAllocator());
    }

    void Character::process(const float alpha) {
        Node::process(alpha);
        updating = true;
        // const auto position = virtualCharacter->GetPosition();
        // const auto pos = float3{position.GetX(), position.GetY() - yDelta, position.GetZ()};
        // if (any(pos != getPositionGlobal())) {
        //     setPositionGlobal(pos);
        // }
        // const auto rotation = virtualCharacter->GetRotation();
        // const auto rot = quaternion{rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()};
        // if (any(rot != getRotationGlobal())) {
        //     // setRotation(rot);
        // }
        updating = false;
    }

    float3 Character::getVelocity() const {
        // if (getBodyId().IsInvalid()) { return FLOAT3ZERO; }
        // const auto velocity = virtualCharacter->GetLinearVelocity();
        // return float3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
        return FLOAT3ZERO;
    }

    void Character::setCollisionLayer(const uint32_t layer) {
        collisionLayer = layer;
        // auto& engine = dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine());
        // objectLayerFilter = std::make_unique<JPH::DefaultObjectLayerFilter>(
        //     engine.getObjectLayerPairFilter(),
        //     collisionLayer);
    }

}