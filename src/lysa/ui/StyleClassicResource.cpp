/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.style_classic_resource;

import lysa.global;

namespace lysa::ui {

    StyleClassicResource::StyleClassicResource(const std::string &resource) :
        Resource{resource} {
        splitResString(resource);
    }
    void StyleClassicResource::splitResString(const std::string &resource) {
        const auto res = split(resource, ',');
        if ((!res.empty()) && (!res[0].empty())) {
            width = stof(std::string{res[0]});
        }
        if ((res.size() > 1) && (!res[1].empty())) {
            height = stof(std::string{res[1]});
        }
        if (res.size() > 2) {
            if (res[2] == "RAISED") {
                style = RAISED;
            } else if (res[2] == "LOWERED") {
                style = LOWERED;
            } else if (res[2] == "FLAT") {
                style = FLAT;
            }
        }
        if (res.size() > 3) {
            color       = float4{stof(std::string{res[3]}), stof(std::string{res[4]}), stof(std::string{res[5]}), stof(std::string{res[6]})};
            customColor = true;
        }
    }

}