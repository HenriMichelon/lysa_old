/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.physics.physx.engine;

import lysa.application;
import lysa.global;
import lysa.nodes.collision_object;
import lysa.nodes.node;

namespace lysa {

    PhysXPhysicsEngine::PhysXPhysicsEngine(const LayerCollisionTable& layerCollisionTable) {
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
        if (!foundation) {
            throw Exception("Failed to create PhysX foundation");
        }
        physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale());
        if (!foundation) {
            throw Exception("Failed to create PhysX physics");
        }

    }

    PhysXPhysicsEngine::~PhysXPhysicsEngine() {
        physics->release();
        foundation->release();
    }


    void PhysXPhysicsEngine::update(const float deltaTime) {
    }

    float3 PhysXPhysicsEngine::getGravity() const {
    }

}