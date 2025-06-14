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

export namespace lysa {

    class PhysXPhysicsScene : public PhysicsScene {
    public:
        PhysXPhysicsScene(physx::PxPhysics*);

        void update(float deltaTime) override;

        float3 getGravity() const override;

        ~PhysXPhysicsScene() override;
    private:
        physx::PxScene* scene;
    };


    class PhysXPhysicsEngine : public PhysicsEngine {
    public:
        PhysXPhysicsEngine(const LayerCollisionTable& layerCollisionTable);

        std::unique_ptr<PhysicsScene> createScene() override;

        auto getPhysics() const { return physics; }

        ~PhysXPhysicsEngine() override;

    private:
        physx::PxDefaultAllocator      gAllocator;
        physx::PxDefaultErrorCallback  gErrorCallback;
        physx::PxFoundation*           foundation;
        physx::PxPhysics*              physics;
    };

}