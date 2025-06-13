/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
module lysa.nodes.rigid_body;

import lysa.nodes.node;

namespace lysa {

    std::shared_ptr<Node> RigidBody::duplicateInstance() const {
        return std::make_shared<RigidBody>(*this);
    }

}
