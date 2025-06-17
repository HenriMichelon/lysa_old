/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.rigid_body;

import lysa.constants;

namespace lysa {

    RigidBody::RigidBody(const std::shared_ptr<Shape>& shape,
                         const collision_layer layer,
                         const std::wstring& name):
        PhysicsBody(shape,
                    layer,
                    physx::PxActorType::eRIGID_DYNAMIC,
                    name,
                    RIGID_BODY) {
    }

    RigidBody::RigidBody(const std::wstring& name):
        PhysicsBody(0,
                    physx::PxActorType::eRIGID_DYNAMIC,
                    name,
                    RIGID_BODY) {
    }

    void RigidBody::createBody(const std::shared_ptr<Shape> &shape) {
        PhysicsBody::createBody(shape);
        setMass(mass);
    }

    void RigidBody::setDensity(const float density) {
        if (!actor || !scene) return;
        const auto body = static_cast<physx::PxRigidDynamic*>(actor);
        physx::PxRigidBodyExt::updateMassAndInertia(*body, density);
    }


    void RigidBody::setVelocity(const float3& velocity) {
        if (!actor || !scene) return;
        const auto body = static_cast<physx::PxRigidDynamic*>(actor);
        if (all(velocity == FLOAT3ZERO)) {
            body->setLinearVelocity(physx::PxVec3(0, 0, 0));
        } else {
            const float3 v = mul(velocity, getRotationGlobal());
            body->setLinearVelocity(physx::PxVec3(v.x, v.y, v.z));
        }
    }

    float3 RigidBody::getVelocity() const {
        if (!actor || !scene) return FLOAT3ZERO;
        const auto v = static_cast<physx::PxRigidDynamic*>(actor)->getLinearVelocity();
        return float3{v.x, v.y, v.z};
    }

    void RigidBody::setGravityFactor(const float factor) {
        gravityFactor = factor;
        if (!actor || !scene) return;
        const auto body = static_cast<physx::PxRigidDynamic*>(actor);
        body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, gravityFactor == 0.0f);
    }


    void RigidBody::applyForce(const float3& force) const {
        if (!actor || !scene) return;
        static_cast<physx::PxRigidDynamic*>(actor)->addForce(physx::PxVec3(force.x, force.y, force.z));
    }

    void RigidBody::applyForce(const float3& force, const float3&position) const {
        if (!actor || !scene) return;
        const auto body = static_cast<physx::PxRigidDynamic*>(actor);
        physx::PxRigidBodyExt::addForceAtPos(
            *body,
            physx::PxVec3(force.x, force.y, force.z),
            physx::PxVec3(position.x, position.y, position.z));
    }

    void RigidBody::setMass(const float value) {
        mass = value;
        if (!actor || !scene) return;
        auto* body = static_cast<physx::PxRigidDynamic*>(actor);
        if (mass > 0.0f) {
            physx::PxRigidBodyExt::setMassAndUpdateInertia(*body, mass);
        } else {
            physx::PxRigidBodyExt::updateMassAndInertia(*body, density);
        }
    }

}
