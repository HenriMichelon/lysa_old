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
export module lysa.physics.physics_material;

import std;

export namespace lysa {

    class PhysicsMaterial
#ifdef PHYSIC_ENGINE_JOLT
        : JPH::PhysicsMaterial
#endif
    {
    public:
        PhysicsMaterial* duplicate() const;

#ifdef PHYSIC_ENGINE_JOLT
        PhysicsMaterial(
            float staticFriction = 0.5f,
            float dynamicFriction = 0.5f,
            float restitution = 0.0f);

        float staticFriction;
        float dynamicFriction;
        float restitution;
#endif
    };

}