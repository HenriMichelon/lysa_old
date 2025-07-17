/*
 * Copyright (c) 2024-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
module lysa.nodes.physics_body;

import lysa.global;
import lysa.viewport;
import lysa.physics.jolt.engine;

namespace lysa {

    PhysicsBody::PhysicsBody(const std::shared_ptr<Shape>& shape,
                             const collision_layer layer,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType,
                             const std::wstring& name,
                             const Type type):
        CollisionObject{shape, layer, name, type},
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
        this->shape = shape;
        joltShape = shape->getShapeSettings()->Create().Get();
    }

    void PhysicsBody::createBody(const std::shared_ptr<Shape> &shape) {
        const auto &position = getPositionGlobal();
        const auto &quat = normalize(getRotationGlobal());
        const JPH::BodyCreationSettings settings{
            joltShape,
            JPH::RVec3{position.x, position.y, position.z},
            JPH::Quat{quat.x, quat.y, quat.z, quat.w},
            motionType,
            collisionLayer,
        };
        // const auto start = std::chrono::high_resolution_clock::now();
        const auto body = bodyInterface->CreateBody(settings);
        // auto end = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double, std::milli> duration = end - start;
        // if (duration.count() > 10.) {
            // std::printf("CreateBody %f\n", duration.count());
        // }
        setBodyId(body->GetID());
        const auto scale = getScale();
        if (any(scale != float3{1.0f, 1.0f, 1.0f})) {
            bodyInterface->SetShape(
                bodyId,
                new JPH::ScaledShape(
                    bodyInterface->GetShape(bodyId),
                    JPH::Vec3{scale.x, scale.y, scale.z}),
                true,
                activationMode);
        }
    }

}
