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
    /**
     * Physics material used by the Jolt backend.
     *
     * Stores coefficients affecting contact response between two shapes and how
     * restitution is combined. This derives from JPH::PhysicsMaterial so it can
     * be attached directly to Jolt shapes.
     */
    class PhysicsMaterial : JPH::PhysicsMaterial
    {
    public:
        /**
         * Constructs a Jolt physics material.
         * @param friction    Surface friction coefficient (>= 0).
         * @param restitution Coefficient of restitution (bounciness) in [0..1].
         */
        PhysicsMaterial(
            float friction,
            float restitution);

        /** Friction coefficient used for Coulomb friction. */
        float friction;
        /** Restitution (bounciness) used for collisions. */
        float restitution;
        /**
         * How restitution is combined when two materials interact.
         * The specific semantics of CombineMode are defined in lysa.enums.
         */
        CombineMode restitutionCombineMode{CombineMode::MAX};
    };
#endif

#ifdef PHYSIC_ENGINE_PHYSX
    /**
     * Physics material type used when the PhysX backend is active.
     *
     * This is an alias to physx::PxMaterial so that higher‑level code can use
     * PhysicsMaterial uniformly across backends.
     */
    using PhysicsMaterial = physx::PxMaterial;
#endif
}