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
        physx::PxCapsuleControllerDesc desc;
        desc.height = height - 2.0f * radius;
        desc.radius = radius;
        desc.position = physx::PxExtendedVec3(position.x, position.y, position.z);
        desc.upDirection = physx::PxVec3(upVector.x, upVector.y, upVector.z);
        desc.slopeLimit = physx::PxCos(radians(maxSlopeAngle));
        desc.stepOffset = 0.3f;
        desc.contactOffset = 0.1f;
        desc.material = getPhysx()->createMaterial(0.5f, 0.5f, 0.0f);
        desc.reportCallback = nullptr; // XXX
        desc.behaviorCallback = nullptr;
        capsuleController = static_cast<physx::PxCapsuleController*>(
            dynamic_cast<PhysXPhysicsScene&>(getViewport()->getPhysicsScene()).getControllerManager()->createController(desc));
        controller = capsuleController;
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
        if (controller) {
            controller->setUpDirection(physx::PxVec3(upVector.x, upVector.y, upVector.z));
        }
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

    void Character::setVelocity(const float3& velocity) {
        this->velocity = velocity;
    }

    void Character::setMaxSlopeAngle(const float angle) {
        maxSlopeAngle = angle;
    }

    void Character::setPositionAndRotation() {
        if (updating || !controller) {
            return;
        }
        const float3 pos = getPositionGlobal();
        controller->setPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
    }

    void Character::physicsProcess(const float delta) {
        Node::physicsProcess(delta);
        if (!controller) return;
        auto disp = velocity * delta;
        physx::PxFilterData filterData;
        filterData.word0 = collisionLayer;
        physx::PxControllerFilters filters;
        filters.mFilterData = &filterData;
        controller->move(physx::PxVec3(disp.x, disp.y, disp.z), 0.001f, delta, filters);
    }

    void Character::process(const float alpha) {
        Node::process(alpha);
        if (!controller) { return; }
        updating = true;
        const physx::PxExtendedVec3 position = controller->getPosition();
        const auto pos = float3{
            static_cast<float>(position.x),
            static_cast<float>(position.y),
            static_cast<float>(position.z)
        };
        if (any(pos != getPositionGlobal())) {
            setPositionGlobal(pos);
        }
        updating = false;
    }

    float3 Character::getVelocity() const {
        return velocity;
    }

    void Character::setCollisionLayer(const uint32_t layer) {
        collisionLayer = layer;
    }

}