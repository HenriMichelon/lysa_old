/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.environment;

namespace lysa {

    Environment::Environment(
            const float4    colorAndIntensity,
            const std::string &nodeName):
        Node{nodeName, ENVIRONMENT},
        ambientColorIntensity{colorAndIntensity} {
    }

    void Environment::setProperty(const std::string &property, const std::string &value) {
        Node::setProperty(property, value);
        if (property == "ambient_color") {
            setAmbientColorAndIntensity(to_float4(value));
        }
    }

    std::shared_ptr<Node> Environment::duplicateInstance() const {
        return std::make_shared<Environment>(*this);
    }

}
