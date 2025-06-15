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
import lysa.physics.physx.engine;

namespace lysa {

    // PhysicsBody::PhysicsBody(const std::shared_ptr<Shape>& shape,
    //                          const collision_layer layer,
    //                          const JPH::EActivation activationMode,
    //                          const JPH::EMotionType motionType,
    //                          const std::wstring& name,
    //                          const Type type):
    //     CollisionObject{shape, layer, name, type},
    //     motionType{motionType} {
    //     this->activationMode = activationMode;
    // }
    //
    // PhysicsBody::PhysicsBody(const collision_layer layer,
    //                          const JPH::EActivation activationMode,
    //                          const JPH::EMotionType motionType,
    //                          const std::wstring& name,
    //                          const Type type):
    //     CollisionObject{layer, name, type},
    //     motionType{motionType} {
    //     this->activationMode = activationMode;
    // }

    void PhysicsBody::setShape(const std::shared_ptr<Shape> &shape) {
        releaseResources();
        const auto &position = getPositionGlobal();
        const auto &quat = normalize(getRotationGlobal());
        this->shape = shape;
        // const JPH::BodyCreationSettings settings{
        //         reinterpret_cast<JPH::ShapeSettings*>(shape->getShapeHandle()),
        //         JPH::RVec3{position.x, position.y, position.z},
        //         JPH::Quat{quat.x, quat.y, quat.z, quat.w},
        //         motionType,
        //         collisionLayer,
        // };
        // const auto body = bodyInterface->CreateBody(settings);
        // setBodyId(body->GetID());
    }

    void PhysicsBody::setVelocity(const float3& velocity) {
        // if (bodyId.IsInvalid() || !bodyInterface) { return; }
        // if (all(velocity == FLOAT3ZERO)) {
        //     bodyInterface->SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        // } else {
        //     // current orientation * velocity
        //     const auto vel = mul(velocity, getRotationGlobal());
        //     bodyInterface->SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
        // }
    }

    void PhysicsBody::setGravityFactor(const float factor) {
        gravityFactor = factor;
        // if (bodyId.IsInvalid() || !bodyInterface) { return; }
        // bodyInterface->SetGravityFactor(bodyId, factor);
    }

    float3 PhysicsBody::getVelocity() const {
        // if (bodyId.IsInvalid() || !bodyInterface) { return FLOAT3ZERO; }
        // const auto velocity = bodyInterface->GetLinearVelocity(bodyId);
        // return float3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
        return FLOAT3ZERO;
    }

    void PhysicsBody::applyForce(const float3& force) const {
        // if (bodyId.IsInvalid() || !bodyInterface) { return; }
        // bodyInterface->AddForce(
        //         bodyId,
        //         JPH::Vec3{force.x, force.y, force.z});
    }

    void PhysicsBody::applyForce(const float3& force, const float3& position) const {
        // if (bodyId.IsInvalid() || !bodyInterface) { return; }
        // bodyInterface->AddForce(
        //         bodyId,
        //         JPH::Vec3{force.x, force.y, force.z},
        //         JPH::Vec3{position.x, position.y, position.z});
    }

    void PhysicsBody::setMass(const float value) {
        if (getType() == STATIC_BODY) { return; }
        mass = value;
        // if (bodyId.IsInvalid() || !bodyInterface) { return; }
        // const JPH::BodyLockWrite lock(dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene())
        //     .getPhysicsSystem()
        //     .GetBodyLockInterface(),
        //     getBodyId());
        // if (lock.Succeeded()) {
        //     JPH::MotionProperties *mp = lock.GetBody().GetMotionProperties();
        //     if (value != 0.0f) {
        //         mp->SetInverseMass(1.0f/value);
        //     } else {
        //         mp->SetInverseMass(0.0f);
        //     }
        // }
    }

}
