/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.static_body;

import lysa.nodes.node;

namespace lysa {

    StaticBody::StaticBody(const std::shared_ptr<Shape>& shape,
                           const collision_layer layer,
                           const std::string& name):
        PhysicsBody(shape,
                    layer,
                    physx::PxActorType::eRIGID_STATIC,
                    name,
                    STATIC_BODY) {
    }

    StaticBody::StaticBody(const collision_layer layer,
                           const std::string& name):
        PhysicsBody(layer,
                    physx::PxActorType::eRIGID_STATIC,
                    name,
                    STATIC_BODY) {
    }

    StaticBody::StaticBody(const std::string& name):
        PhysicsBody(0,
                    physx::PxActorType::eRIGID_STATIC,
                    name,
                    STATIC_BODY) {
    }

}
