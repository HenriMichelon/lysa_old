/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.ray_cast;

import lysa.application;
import lysa.viewport;
import lysa.physics.physx.engine;

namespace lysa {

    void RayCast::physicsProcess(const float delta) {
        Node::physicsProcess(delta);
        const auto position = getPositionGlobal();
        const auto worldDirection = toGlobal(target) - position;
        // const JPH::RRayCast ray{
        //     JPH::Vec3{position.x, position.y, position.z },
        //     JPH::Vec3{worldDirection.x, worldDirection.y, worldDirection.z}
        // };
        // JPH::RayCastResult result;
        // auto& physicsScene = dynamic_cast<JoltPhysicsScene&>(getViewport()->getPhysicsScene());
        // if (physicsScene.getPhysicsSystem().GetNarrowPhaseQuery().CastRay(
        //         ray,
        //         result,
        //         broadPhaseLayerFilter,
        //         *objectLayerFilter,
        //         *this)) {
        //     const auto obj = reinterpret_cast<CollisionObject *>(
        //            physicsScene.getBodyInterface().GetUserData(result.mBodyID));
        //     collider = obj->sharedPtr();
        //     const auto posInRay = ray.GetPointOnRay(result.mFraction);
        //     hitPoint = float3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
        // } else {
        //     collider = nullptr;
        // }
    }

    void RayCast::setCollisionLayer(const collision_layer layer) {
        // auto& engine = dynamic_cast<JoltPhysicsEngine&>(Application::getPhysicsEngine());
        // collisionLayer = layer;
        // objectLayerFilter = std::make_unique<JPH::DefaultObjectLayerFilter>(
        //     engine.getObjectLayerPairFilter(),
        //     collisionLayer);
    }

}
