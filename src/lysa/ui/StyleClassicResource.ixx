/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.style_classic_resource;

import lysa.math;
import lysa.types;

import lysa.ui.resource;

export namespace lysa::ui {

    class StyleClassicResource : public Resource {
    public:
        enum Style {
            FLAT,
            RAISED,
            LOWERED
        };

        Style style{RAISED};
        float width{0};
        float height{0};
        bool customColor{false};
        float4 color{0.0f};

        explicit StyleClassicResource(const std::string& resource);

    private:
        void splitResString(const std::string& resource);
    };
}
