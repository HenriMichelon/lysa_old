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
import lysa.log;
import lysa.viewport;
import lysa.nodes.node;
import lysa.physics.physx.engine;

namespace lysa {

    void Character::setShape(const float height, const float radius) {
        assert([&]{ return height/2 > radius; }, "Invalid capsule shape: height/2 < radius");
        const auto position= getPositionGlobal();
        physx::PxCapsuleControllerDesc desc;
        desc.height = height - 2.0f * radius;
        desc.radius = radius;
        yDelta = height * 0.5f;
        desc.position = physx::PxExtendedVec3(position.x, position.y + yDelta, position.z);
        desc.upDirection = physx::PxVec3(upVector.x, upVector.y, upVector.z);
        desc.slopeLimit = physx::PxCos(radians(maxSlopeAngle));
        desc.stepOffset = 0.3f;
        desc.contactOffset = 0.05f;
        desc.density = 10.0f;
        desc.material = physX.createMaterial(0.5f, 0.0f);
        desc.reportCallback = this;
        desc.behaviorCallback = nullptr;
        capsuleController = static_cast<physx::PxCapsuleController*>(
            dynamic_cast<PhysXPhysicsScene&>(getViewport()->getPhysicsScene()).getControllerManager()->createController(desc));
        setCollisionLayer(collisionLayer);
        constexpr float scale = 1.2f;
        capsule = physx::PxCapsuleGeometry{
            capsuleController->getRadius() * scale,
            (capsuleController->getHeight() * 0.5f) * scale};
    }

    float3 Character::getGroundVelocity() const {
        if (ground) {
            auto* actor = ground->getActor();
            if (actor) {
                const auto* rigid = actor->is<physx::PxRigidDynamic>();
                if (rigid) {
                    const auto& v = rigid->getLinearVelocity();
                    return float3{v.x, v.y, v.z};
                }
            }
        }
        return FLOAT3ZERO;
    }

    Node* Character::getGround() const {
        return ground;
    }

    bool Character::isOnGround() const {
        physx::PxControllerState state;
        capsuleController->getState(state);
        return onGround && !state.isMovingUp;
    }

    bool Character::isGround(const CollisionObject &object) const {
        return &object == ground;
    }

    void Character::setUp(const float3& vector) {
        upVector = vector;
        if (capsuleController) {
            capsuleController->setUpDirection(physx::PxVec3(upVector.x, upVector.y, upVector.z));
        }
    }

    std::list<CollisionObject::Collision> Character::getCollisions() const {
        return {currentContacts.begin(), currentContacts.end()};
    }

    void Character::setVelocity(const float3& velocity) {
        this->velocity = velocity;
    }

    void Character::setMaxSlopeAngle(const float angle) {
        maxSlopeAngle = angle;
    }

    void Character::setPositionAndRotation() {
        if (updating || !capsuleController) {
            return;
        }
        const float3 pos = getPositionGlobal();
        capsuleController->setPosition(physx::PxExtendedVec3(pos.x, pos.y + yDelta, pos.z));
    }

    void Character::physicsProcess(const float delta) {
        Node::physicsProcess(delta);
        if (!capsuleController) return;

        const auto pos = capsuleController->getPosition();
        const auto origin = physx::PxVec3 {
            static_cast<float>(pos.x),
            static_cast<float>(pos.y),
            static_cast<float>(pos.z)
        };
        physx::PxOverlapHit hitBuffer[16];
        physx::PxOverlapBuffer buffer(hitBuffer, 16);
        auto filterData = physx::PxQueryFilterData {};
        filterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;
        std::unordered_set<CollisionObject*> contacts;
        if (scene->overlap(capsule, physx::PxTransform {origin}, buffer, filterData)) {
            for (int i = 0; i < buffer.nbTouches; i++) {
                const auto actor = buffer.getTouch(i).actor;
                if (actor && actor->userData) {
                    contacts.insert(static_cast<CollisionObject*>(actor->userData));
                }
            }
        }
        currentContacts.remove_if([&](const auto& contact) {
            return !contacts.contains(contact.object);
        });

        const auto disp = velocity * delta;
        float3 globalDir = mul(disp, float3x3{globalTransform});

        physx::PxControllerFilters filters;
        filters.mFilterData = nullptr;
        filters.mFilterCallback = this;
        const auto flag =
            capsuleController->move(
                physx::PxVec3(globalDir.x, globalDir.y, globalDir.z),
                0.001f,
                delta,
                filters);
        onGround = flag.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN) && ground;
    }

    void Character::process(const float alpha) {
        Node::process(alpha);
        if (!capsuleController) { return; }
        updating = true;
        const physx::PxExtendedVec3 position = capsuleController->getPosition();
        const auto pos = float3{
            static_cast<float>(position.x),
            static_cast<float>(position.y) - yDelta,
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

    physx::PxQueryHitType::Enum Character::preFilter(
        const physx::PxFilterData &filterData,
        const physx::PxShape *shape,
        const physx::PxRigidActor *actor,
        physx::PxHitFlags &queryFlags) {
        const auto shapeData = shape->getQueryFilterData();
        if (physX.shouldCollide(collisionLayer, shapeData.word0)) {
            return physx::PxQueryHitType::eBLOCK;
        }
        return physx::PxQueryHitType::eNONE;
    }

    physx::PxQueryHitType::Enum Character::postFilter(
        const physx::PxFilterData &filterData,
        const physx::PxQueryHit &hit,
        const physx::PxShape *shape,
        const physx::PxRigidActor *actor) {
        return physx::PxQueryHitType::eBLOCK;
    }

    void Character::onShapeHit(const physx::PxControllerShapeHit &hit) {
        const auto node = static_cast<CollisionObject*>(hit.actor->userData);
        if (node) {
            const auto normal = float3{hit.worldNormal.x, hit.worldNormal.y, hit.worldNormal.z};
            const auto position = float3{hit.worldPos.x, hit.worldPos.y, hit.worldPos.z};
            currentContacts.push_back(Collision{
                .position = position,
                .normal = normal,
                .object = node
            });
            if (normal.y > 0.0f) {
                ground = node;
            }
        }
    }

    void Character::onControllerHit(const physx::PxControllersHit &hit) {
    }

    void Character::onObstacleHit(const physx::PxControllerObstacleHit &hit) {
    }
}
