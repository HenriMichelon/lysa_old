/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.physics_body;

import lysa.global;
import lysa.viewport;
import lysa.resources.static_compound_shape;
import lysa.physics.physx.engine;

namespace lysa {

    PhysicsBody::PhysicsBody(
        const std::shared_ptr<Shape>& shape,
        const collision_layer layer,
        const physx::PxActorType::Enum actorType,
        const std::wstring& name,
        const Type type):
        CollisionObject{shape, layer, name, type},
        actorType{actorType} {
    }

    PhysicsBody::PhysicsBody(
        const collision_layer layer,
        const physx::PxActorType::Enum actorType,
        const std::wstring& name,
        const Type type):
        CollisionObject{layer, name, type},
        actorType{actorType} {
    }

    void PhysicsBody::setShape(const std::shared_ptr<Shape> &shape) {
        if (this->shape) {
            releaseResources();
        }
        this->shape = shape;
        const auto& physx = getPhysx();

        const auto &position = getPositionGlobal();
        const auto &quat = normalize(getRotationGlobal());
        const physx::PxTransform transform{
            physx::PxVec3(position.x, position.y, position.z),
            physx::PxQuat(quat.x, quat.y, quat.z, quat.w)};
        if (actorType == physx::PxActorType::eRIGID_DYNAMIC) {
            physx::PxRigidDynamic* body = physx->createRigidDynamic(transform);
            if (mass > 0.0f) {
                body->setMass(mass);
            } else {
                physx::PxRigidBodyExt::updateMassAndInertia(*body, /*density=*/1000.0f);
            }
            body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, gravityFactor == 0.0f);
            body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, getType() == KINEMATIC_BODY);
            // body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
            // body->setMaxDepenetrationVelocity(2.0f);
            setActor(body);
        } else {
            setActor(physx->createRigidStatic(transform));
        }
        actor->setActorFlag(physx::PxActorFlag::eVISUALIZATION, 1.0f);

        if (const auto& compound = std::dynamic_pointer_cast<StaticCompoundShape>(shape)) {
            for (const auto& subshape : compound->getSubShapes()) {
                auto pxShape = physx->createShape(subshape.shape->getGeometry(), subshape.shape->getMaterial(), true);
                pxShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, 1.0f);
                const auto &localPos = subshape.position;
                const auto &localQuat = normalize(subshape.rotation);
                const physx::PxTransform localPose{
                    physx::PxVec3(localPos.x, localPos.y, localPos.z),
                    physx::PxQuat(localQuat.x, localQuat.y, localQuat.z, localQuat.w)};
                pxShape->setLocalPose(localPose);
                // pxShape->setContactOffset(0.001f);
                // pxShape->setRestOffset(0.0f);
                shapes.push_back(pxShape);
                actor->attachShape(*pxShape);
            }
        } else {
            const auto pxShape = physx->createShape(shape->getGeometry(), shape->getMaterial(), true);
            pxShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, 1.0f);
            shapes.push_back(pxShape);
            // pxShape->setContactOffset(0.001f);
            // pxShape->setRestOffset(0.0f);
            actor->attachShape(*pxShape);
        }
    }

    void PhysicsBody::setVelocity(const float3& velocity) {
        if (!actor || !scene || actorType != physx::PxActorType::eRIGID_DYNAMIC) return;
        const auto body = static_cast<physx::PxRigidDynamic*>(actor);
        if (all(velocity == FLOAT3ZERO)) {
            body->setLinearVelocity(physx::PxVec3(0, 0, 0));
        } else {
            const float3 v = mul(velocity, getRotationGlobal());
            body->setLinearVelocity(physx::PxVec3(v.x, v.y, v.z));
        }
    }

    float3 PhysicsBody::getVelocity() const {
        if (!actor || !scene || actorType != physx::PxActorType::eRIGID_DYNAMIC) return FLOAT3ZERO;
        const auto v = static_cast<physx::PxRigidDynamic*>(actor)->getLinearVelocity();
        return float3{v.x, v.y, v.z};
    }

    void PhysicsBody::setGravityFactor(const float factor) {
        gravityFactor = factor;
        if (!actor || !scene || actorType != physx::PxActorType::eRIGID_DYNAMIC) return;
        const auto body = static_cast<physx::PxRigidDynamic*>(actor);
        body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, gravityFactor == 0.0f);
    }


    void PhysicsBody::applyForce(const float3& force) const {
        if (!actor || !scene || actorType != physx::PxActorType::eRIGID_DYNAMIC) return;
        static_cast<physx::PxRigidDynamic*>(actor)->addForce(physx::PxVec3(force.x, force.y, force.z));
    }

    void PhysicsBody::applyForce(const float3& force, const float3&) const {
        const auto body = static_cast<physx::PxRigidDynamic*>(actor);
        physx::PxRigidBodyExt::addForceAtPos(
            *body,
            physx::PxVec3(force.x, force.y, force.z),
            physx::PxVec3(0, 0, 0));
    }

    void PhysicsBody::setMass(const float value) {
        mass = value;
        if (!actor || !scene || actorType != physx::PxActorType::eRIGID_DYNAMIC) return;
        auto* body = static_cast<physx::PxRigidDynamic*>(actor);
        body->setMass(mass);
    }

}
