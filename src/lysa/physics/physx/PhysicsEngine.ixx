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
import lysa.math;
import lysa.physics.configuration;
import lysa.physics.engine;
import lysa.physics.physics_material;
import lysa.renderers.debug;

export namespace lysa {

    class PhysXPhysicsScene : public PhysicsScene {
    public:
        PhysXPhysicsScene(physx::PxPhysics*);

        void update(float deltaTime) override;

        void debug(DebugRenderer& debugRenderer) override;

        float3 getGravity() const override;

        auto getScene() const { return scene; }

        ~PhysXPhysicsScene() override;
    private:
        physx::PxScene* scene;

        static float4 colorU32ToFloat4(physx::PxU32 color);
    };


    class PhysXPhysicsEngine : public PhysicsEngine {
    public:
        PhysXPhysicsEngine();

        std::unique_ptr<PhysicsScene> createScene() override;

        PhysicsMaterial* createMaterial(
            float friction = 0.0f,
            float restitution = 0.0f) const override;

        PhysicsMaterial* duplicateMaterial(const PhysicsMaterial* orig) const override;

        void setRestitutionCombineMode(PhysicsMaterial* physicsMaterial, CombineMode combineMode) const override;

        auto getPhysics() const { return physics; }

        ~PhysXPhysicsEngine() override;

    private:
        physx::PxDefaultAllocator      gAllocator;
        physx::PxDefaultErrorCallback  gErrorCallback;
        physx::PxFoundation*           foundation;
        physx::PxPhysics*              physics;
    };

}