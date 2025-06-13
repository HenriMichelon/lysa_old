/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
module lysa.nodes.character;

import lysa.application;
import lysa.global;
import lysa.nodes.node;
import lysa.physics.jolt.engine;

namespace lysa {
    void Character::setShape(const float height, const float radius) {
        assert([&]{ return height/2 > radius; }, "Invalid capsule shape: height/2 < radius");
        yDelta = height/2;
        const auto position= getPositionGlobal();
        const auto quat = normalize(getRotationGlobal());
        const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        auto& engine = dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine());

        JPH::CharacterVirtualSettings settingsVirtual;
        settingsVirtual.mShape          = new JPH::CapsuleShape(height/2 - radius, radius);
        settingsVirtual.mInnerBodyLayer = collisionLayer;
        settingsVirtual.mInnerBodyShape = settingsVirtual.mShape;
        settingsVirtual.mEnhancedInternalEdgeRemoval = true;
        settingsVirtual.mUp = JPH::Vec3{upVector.x, upVector.y, upVector.z};
        virtualCharacter = std::make_unique<JPH::CharacterVirtual>(&settingsVirtual,
                                                       pos,
                                                       rot,
                                                       reinterpret_cast<uint64>(this),
                                                       &engine.getPhysicsSystem());
        setCollisionLayer(collisionLayer);
        virtualCharacter->SetListener(this);
    }

    float3 Character::getGroundVelocity() const {
        const auto velocity = virtualCharacter->GetGroundVelocity();
        return float3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    Node* Character::getGround() const {
        return reinterpret_cast<Node*>(virtualCharacter->GetGroundUserData());
    }

    bool Character::isOnGround() const {
        return virtualCharacter->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
    }

    bool Character::isGround(const CollisionObject &object) const {
        return object.getBodyId() == virtualCharacter->GetGroundBodyID();
    }

    void Character::setUp(const float3& vector) {
        upVector = vector;
        virtualCharacter->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
    }

    std::list<CollisionObject::Collision> Character::getCollisions() const {
        std::list<Collision> contacts;
        for (const auto &contact : virtualCharacter->GetActiveContacts()) {
            auto *node = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(contact.mBodyB));
            assert([&] { return node; }, "physics body not associated with a node");
            contacts.push_back({
                .position = float3{contact.mPosition.GetX(), contact.mPosition.GetY(), contact.mPosition.GetZ()},
                .normal = float3{contact.mSurfaceNormal.GetX(),
                               contact.mSurfaceNormal.GetY(),
                               contact.mSurfaceNormal.GetZ()},
                .object = node
            });
        }
        return contacts;
    }

    void Character::setVelocity(const float3& velocity) {
        if (all(velocity == FLOAT3ZERO)) {
            virtualCharacter->SetLinearVelocity(JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = mul(getRotationGlobal(), velocity);
            virtualCharacter->SetLinearVelocity(JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    void Character::setMaxSlopeAngle(const float angle) const {
        virtualCharacter->SetMaxSlopeAngle(radians(angle));
    }

    void Character::setPositionAndRotation() {
        if (updating) {
            return;
        }
        const auto position = getPositionGlobal();
        const auto quat = normalize(getRotationGlobal());
        const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        virtualCharacter->SetPosition(pos);
        virtualCharacter->SetRotation(rot);
    }

    void Character::physicsProcess(const float delta) {
        Node::physicsProcess(delta);
        auto& engine = dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine());
        virtualCharacter->Update(delta,
              virtualCharacter->GetUp() * engine.getPhysicsSystem().GetGravity().Length(),
              *this,
              *objectLayerFilter,
              *this,
              {},
              *engine.getTempAllocator().get());
    }

    void Character::process(const float alpha) {
        Node::process(alpha);
        updating = true;
        const auto position = virtualCharacter->GetPosition();
        const auto pos = float3{position.GetX(), position.GetY() - yDelta, position.GetZ()};
        if (any(pos != getPositionGlobal())) {
            setPositionGlobal(pos);
        }
        const auto rotation = virtualCharacter->GetRotation();
        const auto rot = quaternion{rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()};
        if (any(rot != getRotationGlobal())) {
            setRotation(rot);
        }
        updating = false;
    }

    void Character::OnContactAdded(const JPH::CharacterVirtual *  inCharacter,
                                   const JPH::BodyID &            inBodyID2,
                                   const JPH::SubShapeID &        inSubShapeID2,
                                   JPH::RVec3Arg                  inContactPosition,
                                   JPH::Vec3Arg                   inContactNormal,
                                   JPH::CharacterContactSettings &ioSettings) {
        auto *node   = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID2));
        assert([&] { return node; }, "physics body not associated with a node");
        auto event = Collision{
            .position = float3{inContactPosition.GetX(), inContactPosition.GetY(), inContactPosition.GetZ()},
            .normal = float3{inContactNormal.GetX(), inContactNormal.GetY(), inContactNormal.GetZ()},
            .object = node
        };
        // log("Character::OnContactAdded", on_collision, node->getName());
        Application::callDeferred([this, event]{
            this->emit(on_collision, (void*)&event);
        });
    }

    bool Character::OnContactValidate(const JPH::CharacterVirtual *  inCharacter,
                                    const JPH::BodyID &            inBodyID2,
                                    const JPH::SubShapeID &        inSubShapeID2) {
        const auto *node = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID2));
        return isProcessed() && node->isProcessed();
    }

    bool Character::ShouldCollide(const JPH::BodyID &inBodyID) const {
        if (!isProcessed()) { return false; }
        const auto node1 = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID));
        return objectLayerFilter->ShouldCollide(node1->getCollisionLayer());
    }

    bool Character::ShouldCollideLocked(const JPH::Body &inBody) const {
        if (!isProcessed()) { return false; }
        const auto node1 = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        return objectLayerFilter->ShouldCollide(node1->getCollisionLayer());
    }

    float3 Character::getVelocity() const {
        if (getBodyId().IsInvalid()) { return FLOAT3ZERO; }
        const auto velocity = virtualCharacter->GetLinearVelocity();
        return float3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::setCollisionLayer(const uint32_t layer) {
        collisionLayer = layer;
        auto& engine = dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine());
        objectLayerFilter = std::make_unique<JPH::DefaultObjectLayerFilter>(
            engine.getObjectLayerPairFilter(),
            collisionLayer);
    }

}