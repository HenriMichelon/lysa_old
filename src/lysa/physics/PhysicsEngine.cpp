/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.physics.engine;

import lysa.global;
#ifdef PHYSIC_ENGINE_JOLT
import lysa.physics.jolt.engine;
#endif
#ifdef PHYSIC_ENGINE_PHYSX
import lysa.physics.physx.engine;
#endif

namespace lysa {

    std::unique_ptr<PhysicsEngine> PhysicsEngine::create(const PhysicsConfiguration& config) {
#ifdef PHYSIC_ENGINE_JOLT
        return std::make_unique<JoltPhysicsEngine>(config.layerCollisionTable);
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        return std::make_unique<PhysXPhysicsEngine>(config.layerCollisionTable);
#endif
        throw Exception("Not implemented");
    }

}