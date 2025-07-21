/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.spot_light;

import lysa.nodes.node;

namespace lysa {

    SpotLight::SpotLight(const std::string &name):
        OmniLight{name, SPOT_LIGHT} {
    }

    SpotLight::SpotLight(const float cutOffDegrees,
                         const float outerCutOffDegrees,
                         const float range,
                         const float4& color,
                         const std::string &nodeName):
        OmniLight{range, color, nodeName, SPOT_LIGHT},
        fov{radians(outerCutOffDegrees)},
        cutOff{std::cos(radians(cutOffDegrees))},
        outerCutOff{std::cos(fov)} {
    }

    void SpotLight::setCutOff(const float cutOffDegrees) {
        cutOff = std::cos(radians(cutOffDegrees));
    }

    void SpotLight::setOuterCutOff(const float outerCutOffDegrees) {
        fov = radians(outerCutOffDegrees);
        outerCutOff = std::cos(fov);
    }

    LightData SpotLight::getLightData() const {
        auto data = OmniLight::getLightData();
        data.direction = float4{getFrontVector(), 0.0f};
        data.cutOff = cutOff;
        data.outerCutOff = outerCutOff;
        return data;
    }

    std::shared_ptr<Node> SpotLight::duplicateInstance() const {
        return make_shared<SpotLight>(*this);
    }

    void SpotLight::setProperty(const std::string &property, const std::string &value) {
        OmniLight::setProperty(property, value);
        if (property == "cutoff") {
            setCutOff(stof(value));
        } else if (property == "outer_cutoff") {
            setOuterCutOff(stof(value));
        }
    }

}
