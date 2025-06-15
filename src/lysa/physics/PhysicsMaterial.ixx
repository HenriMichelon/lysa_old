/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#endif
#ifdef PHYSIC_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#endif
export module lysa.physics.physics_material;

import std;

export namespace lysa {

#ifdef PHYSIC_ENGINE_JOLT
    class PhysicsMaterial : JPH::PhysicsMaterial
    {
    public:
        PhysicsMaterial(
            float staticFriction = 0.5f,
            float dynamicFriction = 0.5f,
            float restitution = 0.0f);

        float staticFriction;
        float dynamicFriction;
        float restitution;
    };
#endif

#ifdef PHYSIC_ENGINE_PHYSX
    using PhysicsMaterial = physx::PxMaterial;
#endif
}