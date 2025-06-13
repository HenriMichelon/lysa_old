/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.kinematic_body;

import lysa.nodes.node;

namespace lysa {

    std::shared_ptr<Node> KinematicBody::duplicateInstance() const {
        return std::make_shared<KinematicBody>(*this);
    }

}
