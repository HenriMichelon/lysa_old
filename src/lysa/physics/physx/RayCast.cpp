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
import lysa.physics.configuration;
import lysa.physics.physx.engine;

namespace lysa {

    physx::PxQueryHitType::Enum RayCast::preFilter(
        const physx::PxFilterData& filterData,
        const physx::PxShape* shape,
        const physx::PxRigidActor* actor,
        physx::PxHitFlags& queryFlags) {
        if (!actor || !actor->userData) {
            return physx::PxQueryHitType::eNONE;
        }
        const auto node = reinterpret_cast<CollisionObject*>(actor->userData);
        if (!node || !node->isProcessed()) {
            return physx::PxQueryHitType::eNONE;
        }
        if (excludeParent && node == getParent()) {
            return physx::PxQueryHitType::eNONE;
        }

        const auto& config = Application::getConfiguration().physicsConfig;
        const auto targetLayer = node->getCollisionLayer();
        for (const auto& entry : config.layerCollisionTable.layersCollideWith) {
            if (entry.layer == collisionLayer) {
                if (std::ranges::find(entry.collideWith, targetLayer) != entry.collideWith.end()) {
                    return physx::PxQueryHitType::eBLOCK;
                }
                return physx::PxQueryHitType::eNONE;
            }
        }
        return physx::PxQueryHitType::eNONE;
    }

    physx::PxQueryHitType::Enum  RayCast::postFilter(
              const physx::PxFilterData& filterData,
              const physx::PxQueryHit& hit,
              const physx::PxShape* shape,
              const physx::PxRigidActor* actor) {
        return physx::PxQueryHitType::eBLOCK;
    }

    void RayCast::physicsProcess(const float delta) {
        Node::physicsProcess(delta);
        const float3 origin = getPositionGlobal();
        const float3 end = toGlobal(target);
        const float3 dir = normalize(end - origin);
        const float maxDist = length(end - origin);

        physx::PxRaycastBuffer hit;
        physx::PxQueryFilterData filterData;
        filterData.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

        const auto scene = dynamic_cast<PhysXPhysicsScene&>(getViewport()->getPhysicsScene()).getScene();
        bool status = scene->raycast(
            physx::PxVec3(origin.x, origin.y, origin.z),
            physx::PxVec3(dir.x, dir.y, dir.z),
            maxDist,
            hit,
            physx::PxHitFlag::eDEFAULT,
            filterData);

        if (status && hit.hasBlock) {
            physx::PxRigidActor* actor = hit.block.actor;
            if (actor) {
                void* userData = actor->userData;
                auto* obj = reinterpret_cast<CollisionObject*>(userData);
                if (obj && (!excludeParent || obj != getParent()) && isProcessed() && obj->isProcessed()) {
                    collider = obj->sharedPtr();
                    const auto& p = hit.block.position;
                    hitPoint = float3{p.x, p.y, p.z};
                    return;
                }
            }
        }

        collider = nullptr;
    }

    void RayCast::setCollisionLayer(const collision_layer layer) {
        collisionLayer = layer;
    }

}
