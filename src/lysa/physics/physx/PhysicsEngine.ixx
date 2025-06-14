/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
export module lysa.physics.physx.engine;

import lysa.math;
import lysa.physics.configuration;
import lysa.physics.engine;

export namespace lysa {

    class PhysXPhysicsEngine : public PhysicsEngine {
    public:
        PhysXPhysicsEngine(const LayerCollisionTable& layerCollisionTable);

        void update(float deltaTime) override;

        /**
        * Returns the physics system gravity
        */
        float3 getGravity() const override;

        ~PhysXPhysicsEngine() override;

    private:
        physx::PxDefaultAllocator      gAllocator;
        physx::PxDefaultErrorCallback  gErrorCallback;
        physx::PxFoundation*           foundation;
        physx::PxPhysics*              physics;
        physx::PxScene*                scene;
    };

}