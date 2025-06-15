/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.rigid_body;

namespace lysa {

    RigidBody::RigidBody(const std::shared_ptr<Shape>& shape,
                         const collision_layer layer,
                         const std::wstring& name):
        PhysicsBody(shape,
                    layer,
                    physx::PxActorType::eRIGID_DYNAMIC,
                    name,
                    RIGID_BODY) {
    }

    RigidBody::RigidBody(const std::wstring& name):
        PhysicsBody(0,
                    physx::PxActorType::eRIGID_DYNAMIC,
                    name,
                    RIGID_BODY) {
    }

}
