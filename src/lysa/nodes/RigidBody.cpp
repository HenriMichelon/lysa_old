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

    void RigidBody::enterScene() {
        PhysicsBody::enterScene();
        setGravityFactor(gravityFactor);
        if (mass > 0) { setMass(mass); }
    }

    std::shared_ptr<Node> RigidBody::duplicateInstance() const {
        return std::make_shared<RigidBody>(*this);
    }

     void RigidBody::setProperty(const std::string &property, const std::string &value) {
        CollisionObject::setProperty(property, value);
        if (property == "mass") {
            setMass(stof(value));
        }
    }

}
