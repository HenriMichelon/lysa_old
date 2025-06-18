/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/BodyLock.h>
module lysa.nodes.rigid_body;

import lysa.constants;
import lysa.log;
import lysa.physics.jolt.engine;

namespace lysa {

    RigidBody::RigidBody(const std::shared_ptr<Shape>& shape,
                         const collision_layer layer,
                         const std::wstring& name):
        PhysicsBody(shape,
                    layer,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Dynamic,
                    name,
                    RIGID_BODY) {
    }

    RigidBody::RigidBody(const std::wstring& name):
        PhysicsBody(0,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Dynamic,
                    name,
                    RIGID_BODY) {
    }

    void RigidBody::createBody(const std::shared_ptr<Shape> &shape) {
        PhysicsBody::createBody(shape);
        setMass(mass);
    }

    void RigidBody::setVelocity(const float3& velocity) {
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        if (all(velocity == FLOAT3ZERO)) {
            bodyInterface->SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = mul(velocity, getRotationGlobal());
            bodyInterface->SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    void RigidBody::scale(const float scale) {
        PhysicsBody::scale(scale);
        setMass(mass);
    }

    void RigidBody::setGravityFactor(const float factor) {
        gravityFactor = factor;
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        bodyInterface->SetGravityFactor(bodyId, factor);
    }

    float3 RigidBody::getVelocity() const {
        if (bodyId.IsInvalid() || !bodyInterface) { return FLOAT3ZERO; }
        const auto velocity = bodyInterface->GetLinearVelocity(bodyId);
        return float3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void RigidBody::addForce(const float3& force) {
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        bodyInterface->AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z});
    }

    void RigidBody::addForce(const float3& force, const float3& position) {
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        bodyInterface->AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z},
                JPH::Vec3{position.x, position.y, position.z});
    }

    void RigidBody::addImpulse(const float3& force) {
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        bodyInterface->AddImpulse(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z});
    }

    void RigidBody::addImpulse(const float3& force, const float3& position) {
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        bodyInterface->AddImpulse(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z},
                JPH::Vec3{position.x, position.y, position.z});
    }

    void RigidBody::setDensity(const float density) {
        this->density = density;
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        const auto joltShape = bodyInterface->GetShape(bodyId);
        if (joltShape) {
            setMass(joltShape->GetVolume() * density);
            mass = -1.0f;
        }
    }

    void RigidBody::setMass(const float value) {
        mass = value;
        if (bodyId.IsInvalid() || !bodyInterface) { return; }
        if (mass > 0.0f) {
            const JPH::BodyLockWrite lock(dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene())
                .getPhysicsSystem()
                .GetBodyLockInterface(),
                getBodyId());
            if (lock.Succeeded()) {
                JPH::MotionProperties *mp = lock.GetBody().GetMotionProperties();
                if (mass != 0.0f) {
                    mp->SetInverseMass(1.0f/mass);
                } else {
                    mp->SetInverseMass(0.0f);
                }
            }
        } else {
            const auto joltShape = bodyInterface->GetShape(bodyId);
            if (joltShape) {
                setMass(joltShape->GetVolume() * density);
                mass = -1.0f;
            }
        }
    }

    float RigidBody::getMass() const {
        if (bodyId.IsInvalid() || !bodyInterface) { return mass; }
        const JPH::BodyLockWrite lock(dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene())
            .getPhysicsSystem()
            .GetBodyLockInterface(),
            getBodyId());
        if (lock.Succeeded()) {
            const JPH::MotionProperties *mp = lock.GetBody().GetMotionProperties();
            return 1.0f/mp->GetInverseMass();
        }
        return mass;
    }
}
