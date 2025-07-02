/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.directional_light;

import lysa.nodes.node;

namespace lysa {

    DirectionalLight::DirectionalLight(const float4& color,
                                       const std::wstring &nodeName):
        Light{color, nodeName, DIRECTIONAL_LIGHT} {
        shadowMapSize = 2048;
    }

    void DirectionalLight::setShadowMapCascadesCount(const uint32 cascadesCount) {
        shadowMapCascadesCount =
            std::max(static_cast<uint32>(2),
            std::min(cascadesCount, MAX_SHADOW_MAP_CASCADES));
    }

    void DirectionalLight::setProperty(const std::string &property, const std::string &value) {
        Light::setProperty(property, value);
        if (property == "shadow_map_cascade_count") {
            setShadowMapCascadesCount(stoi(value));
        }
    }

    LightData DirectionalLight::getLightData() const {
        auto data = Light::getLightData();
        data.direction = float4{getFrontVector(), 0.0f};
        data.cascadesCount = shadowMapCascadesCount;
        return data;
    }

    std::shared_ptr<Node> DirectionalLight::duplicateInstance() const {
        return std::make_shared<DirectionalLight>(*this);
    }

}
