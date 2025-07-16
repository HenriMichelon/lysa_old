/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.static_body;

import lysa.nodes.node;

namespace lysa {

    std::shared_ptr<Node> StaticBody::duplicateInstance() const {
        auto dup = std::make_shared<StaticBody>(*this);
        dup->recreateBody();
        return dup;
    }

    void StaticBody::process(const float alpha) {
        Node::process(alpha); // static bodies aren't affected by physics
    }

}
