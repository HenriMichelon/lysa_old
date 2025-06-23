/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.omni_light;

import lysa.nodes.node;

namespace lysa {

    OmniLight::OmniLight(const std::wstring &name, const Type type):
        Light{name, type} {
    }

    OmniLight::OmniLight(const float range,
                         const float4& color,
                         const std::wstring& nodeName,
                         const Type type):
        Light{color, nodeName, type}, range{range} {
    }

    void OmniLight::setRange(const float range) {
        this->range = range;
    }

    LightData OmniLight::getLightData() const {
        auto data = Light::getLightData();
        data.range = range;
        return data;
    }

    void OmniLight::setProperty(const std::string &property, const std::string &value) {
        Light::setProperty(property, value);
        if (property == "range") {
            range = stof(value);
        } else if (property == "near") {
            near = stof(value);
        }
    }

    std::shared_ptr<Node> OmniLight::duplicateInstance() const {
        return make_shared<OmniLight>(*this);
    }

}
