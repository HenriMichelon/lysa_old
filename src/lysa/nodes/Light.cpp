/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.light;

namespace lysa {

    Light::Light(const std::string &nodeName, const Type type) :
        Node{nodeName, type},
        lightType{type == DIRECTIONAL_LIGHT ? LIGHT_DIRECTIONAL : type == SPOT_LIGHT ? LIGHT_SPOT : LIGHT_OMNI} {
    }

    Light::Light(const float4& color, const std::string &nodeName, const Type type):
        Node{nodeName, type},
        colorAndIntensity{color},
        lightType{type == DIRECTIONAL_LIGHT ? LIGHT_DIRECTIONAL : type == SPOT_LIGHT ? LIGHT_SPOT : LIGHT_OMNI} {
    }

    void Light::setCastShadows(const bool castShadows) {
        this->castShadows = castShadows;
    }

    LightData Light::getLightData() const {
        return {
            .type = lightType,
            .position = float4{getPositionGlobal(), 0.0f},
            .color = colorAndIntensity
        };
    }

    void Light::setProperty(const std::string &property, const std::string &value) {
        Node::setProperty(property, value);
        if (property == "color") {
            setColorAndIntensity(to_float4(value));
        } else if (property == "cast_shadows") {
            setCastShadows(value == "true");
        } else if (property == "shadowmap_size") {
            setShadowMapSize(std::stoi(value));
        }
    }

}
