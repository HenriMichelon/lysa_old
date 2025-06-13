/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
module lysa.nodes.ray_cast;

import lysa.application;
import lysa.physics.jolt.engine;

namespace lysa {

    void RayCast::physicsProcess(const float delta) {
        Node::physicsProcess(delta);
        const auto position = getPositionGlobal();
        const auto worldDirection = toGlobal(target) - position;
        const JPH::RRayCast ray{
            JPH::Vec3{position.x, position.y, position.z },
            JPH::Vec3{worldDirection.x, worldDirection.y, worldDirection.z}
        };
        JPH::RayCastResult result;
        auto& engine = dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine());
        if (engine.getPhysicsSystem().GetNarrowPhaseQuery().CastRay(
                ray,
                result,
                broadPhaseLayerFilter,
                *objectLayerFilter,
                *this)) {
            const auto obj = reinterpret_cast<CollisionObject *>(
                   engine.getBodyInterface().GetUserData(result.mBodyID));
            collider = obj->sharedPtr();
            const auto posInRay = ray.GetPointOnRay(result.mFraction);
            hitPoint = float3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
        } else {
            collider = nullptr;
        }
    }

    void RayCast::setCollisionLayer(const collision_layer layer) {
        auto& engine = dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine());
        collisionLayer = layer;
        objectLayerFilter = std::make_unique<JPH::DefaultObjectLayerFilter>(
            engine.getObjectLayerPairFilter(),
            collisionLayer);
    }

    bool RayCast::ShouldCollideLocked(const JPH::Body &inBody) const {
        const auto *node = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        return (node != nullptr) && (!(excludeParent && (node == getParent()))) && isProcessed() && node->isProcessed();
    }

}
