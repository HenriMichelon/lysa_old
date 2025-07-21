/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.ray_cast;

namespace lysa {

    RayCast::RayCast(const float3 &target, const collision_layer layer, const std::string &name):
        Node{name, RAYCAST},
        target{target},
        collisionLayer{layer} {
        setCollisionLayer(collisionLayer);
    }

    RayCast::RayCast(const std::string &name):
        Node{name, RAYCAST} {
    }

}
