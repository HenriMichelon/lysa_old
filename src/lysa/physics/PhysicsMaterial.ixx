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
import lysa.enums;

export namespace lysa {

#ifdef PHYSIC_ENGINE_JOLT
    class PhysicsMaterial : JPH::PhysicsMaterial
    {
    public:
        PhysicsMaterial(
            float friction,
            float restitution);

        float friction;
        float restitution;
        CombineMode restitutionCombineMode{CombineMode::MAX};
    };
#endif

#ifdef PHYSIC_ENGINE_PHYSX
    using PhysicsMaterial = physx::PxMaterial;
#endif
}