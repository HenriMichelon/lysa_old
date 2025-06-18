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
        const auto shapeData = shape->getQueryFilterData();
        if (PhysXPhysicsEngine::collisionMatrix[collisionLayer][shapeData.word0]) {
            return physx::PxQueryHitType::eBLOCK;
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
        filterData.flags =
            physx::PxQueryFlag::eSTATIC |
            physx::PxQueryFlag::eDYNAMIC |
            physx::PxQueryFlag::eANY_HIT |
            physx::PxQueryFlag::ePREFILTER;

        collider = nullptr;
        const auto scene = dynamic_cast<PhysXPhysicsScene&>(getViewport()->getPhysicsScene()).getScene();
        bool status = scene->raycast(
            physx::PxVec3(origin.x, origin.y, origin.z),
            physx::PxVec3(dir.x, dir.y, dir.z),
            maxDist,
            hit,
            physx::PxHitFlag::eDEFAULT,
            filterData,
            this);
        if (status && hit.hasBlock) {
            auto* obj = reinterpret_cast<CollisionObject*>(hit.block.actor->userData);
            collider = obj->sharedPtr();
            const auto& p = hit.block.position;
            hitPoint = float3{p.x, p.y, p.z};
        }

    }

    void RayCast::setCollisionLayer(const collision_layer layer) {
        collisionLayer = layer;
    }

}
