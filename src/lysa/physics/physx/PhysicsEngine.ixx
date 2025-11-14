/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
export module lysa.physics.physx.engine;

import std;
import lysa.configuration;
import lysa.constants;
import lysa.math;
import lysa.physics.configuration;
import lysa.physics.engine;
import lysa.physics.physics_material;
import lysa.renderers.debug;

export namespace lysa {

    /**
     * PhysX implementation of PhysicsScene.
     * Wraps a PxScene and provides step/update, debug rendering and gravity query.
     */
    class PhysXPhysicsScene : public PhysicsScene {
    public:
        PhysXPhysicsScene(physx::PxPhysics*, const DebugConfig& debugConfig);

        /** Advances the PhysX simulation by deltaTime seconds. */
        void update(float deltaTime) override;

        /** Emits debug primitives (when enabled) via the provided renderer. */
        void debug(DebugRenderer& debugRenderer) override;

        /** Returns the current gravity vector configured on the scene. */
        float3 getGravity() const override;

        /** Exposes the underlying PxScene pointer for low‑level operations. */
        auto getScene() const { return scene; }

        /** Returns the character controller manager associated with the scene. */
        auto getControllerManager() const { return controllerManager; }

        /** Destroys PhysX objects owned by the scene. */
        ~PhysXPhysicsScene() override;
    private:
        physx::PxScene* scene;
        physx::PxControllerManager* controllerManager;

        static float4 colorU32ToFloat4(physx::PxU32 color);
    };


    /**
     * PhysX implementation of PhysicsEngine.
     * Responsible for creating scenes/materials and maintaining global PhysX state.
     */
    class PhysXPhysicsEngine : public PhysicsEngine {
    public:
        PhysXPhysicsEngine(const LayerCollisionTable& layerCollisionTable);

        std::unique_ptr<PhysicsScene> createScene(const DebugConfig& debugConfig) override;

        /** Creates a PhysX material with the provided coefficients. */
        PhysicsMaterial* createMaterial(
            float friction = 0.5f,
            float restitution = 0.0f) const override;

        /** Returns a new PxMaterial with the same parameters as orig. */
        PhysicsMaterial* duplicateMaterial(const PhysicsMaterial* orig) const override;

        /** Sets how restitution is combined when two materials interact. */
        void setRestitutionCombineMode(PhysicsMaterial* physicsMaterial, CombineMode combineMode) const override;

        /** Quick predicate using the precomputed collision matrix. */
        inline bool shouldCollide(const uint32_t layer1, const uint32_t layer2) const {
            return collisionMatrix[layer1][layer2];
        }

        /** Returns the global PxPhysics object. */
        auto getPhysics() const { return physics; }

        /** Destroys global PhysX state (foundation, physics, etc.). */
        ~PhysXPhysicsEngine() override;

        static bool collisionMatrix[MAX_COLLISIONS_LAYERS][MAX_COLLISIONS_LAYERS];

    private:
        physx::PxDefaultAllocator     gAllocator;
        physx::PxDefaultErrorCallback gErrorCallback;
        physx::PxFoundation*          foundation;
        physx::PxPhysics*             physics;
    };

}