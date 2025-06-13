/*
 * Copyright (c) 2024-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/EActivation.h>
module lysa.nodes.physics_body;

import lysa.application;
import lysa.global;
import lysa.physics.jolt.engine;

namespace lysa {

    PhysicsBody::PhysicsBody(const std::shared_ptr<Shape>& shape,
                             const collision_layer layer,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType,
                             const std::wstring& name,
                             const Type type):
        CollisionObject{nullptr, layer, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
        setShape(shape);
    }

    PhysicsBody::PhysicsBody(const collision_layer layer,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType,
                             const std::wstring& name,
                             const Type type):
        CollisionObject{layer, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
    }

    void PhysicsBody::setShape(const std::shared_ptr<Shape> &shape) {
        releaseResources();
        const auto &position = getPositionGlobal();
        const auto &quat = normalize(getRotationGlobal());
        this->shape = shape;
        const JPH::BodyCreationSettings settings{
                reinterpret_cast<JPH::ShapeSettings*>(shape->getShapeHandle()),
                JPH::RVec3{position.x, position.y, position.z},
                JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                motionType,
                collisionLayer,
        };
        const auto body = bodyInterface.CreateBody(settings);
        setBodyId(body->GetID());
    }

    void PhysicsBody::setVelocity(const float3& velocity) {
        if (bodyId.IsInvalid()) { return; }
        if (all(velocity == FLOAT3ZERO)) {
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = mul(getRotationGlobal(), velocity);
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    void PhysicsBody::setGravityFactor(const float factor) {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.SetGravityFactor(bodyId, factor);
    }

    float3 PhysicsBody::getVelocity() const {
        if (bodyId.IsInvalid()) { return FLOAT3ZERO; }
        const auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return float3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void PhysicsBody::applyForce(const float3& force) const {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z});
    }

    void PhysicsBody::applyForce(const float3& force, const float3& position) const {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z},
                JPH::Vec3{position.x, position.y, position.z});
    }

    void PhysicsBody::setMass(const float value) const {
        assert([&] { return !getBodyId().IsInvalid();}, "Invalid body id");
        const JPH::BodyLockWrite lock(dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine())
            .getPhysicsSystem()
            .GetBodyLockInterface(),
            getBodyId());
        if (lock.Succeeded()) {
            JPH::MotionProperties *mp = lock.GetBody().GetMotionProperties();
            if (value != 0.0f) {
                mp->SetInverseMass(1.0f/value);
            } else {
                mp->SetInverseMass(0.0f);
            }
        }
    }

    void PhysicsBody::setBounce(const float value) const {
        assert([&] { return !getBodyId().IsInvalid();}, "Invalid body id");
        bodyInterface.SetRestitution(getBodyId(), value);
    }

}
