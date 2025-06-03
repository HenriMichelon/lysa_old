/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.light;

import lysa.global;
import lysa.nodes.node;

export namespace lysa {

    struct LightData {
        // light params
        int32 type{0}; // Light::LightType
        float3 position{0.0f, 0.0f, 0.0f};
        float3 direction{0.0f};
        float4 color{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
        float range{0.0f};
        float cutOff{0.0f};
        float outerCutOff{0.0f};
        // shadow map params
        int32 mapIndex{-1};
        float farPlane{0.0};
        uint32 cascadesCount{0};
        float4 cascadeSplitDepth;
        float4x4 lightSpace[6];
    };

    /**
     * Base class for different kinds of light nodes
     */
    class Light : public Node {
    public:

        /**
         * Light type, mainly used by the fragment shader
         */
        enum LightType {
            LIGHT_UNKNOWN     = -1,
            LIGHT_DIRECTIONAL = 0,
            LIGHT_OMNI        = 1,
            LIGHT_SPOT        = 2
        };

        ~Light() override = default;

        /**
         * Returns the RGB color and the intensity factor
         */
        const auto& getColorAndIntensity() const { return colorAndIntensity; }

        /**
         * Sets the RGB color and the intensity factor
         */
        void setColorAndIntensity(const float4& colorAndIntensity) { this->colorAndIntensity = colorAndIntensity; }

        /**
         * If `true`, the light will cast real-time shadows.<br>
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.
         */
        auto getCastShadows() const { return castShadows; }

        /**
         * Sets to `true` to makes the light cast real-time shadow.<br>
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.<br>
         * Changing this parameter have no effect after adding the light to the scene (to avoid destroying shadow map renderers during frame rendering),
         * you have to remove the light from the scene, change the setting, then add the light to the scene (adding and removing nodes from the
         * scene if a deferred process).
         */
        void setCastShadows(bool castShadows);

        /**
         * Returns the light type
         */
        auto getLightType() const { return lightType; }

        void setProperty(const std::string &property, const std::string &value) override;

        virtual LightData getLightData() const;


    protected:
        Light(const std::wstring &nodeName = TypeNames[LIGHT], Type type = LIGHT);

        Light(const float4& color, const std::wstring &nodeName = TypeNames[LIGHT], Type type = LIGHT);

    private:
        const LightType lightType{LIGHT_UNKNOWN};
        float4 colorAndIntensity{1.0f, 1.0f, 1.0f, 1.0f};
        bool castShadows{false};
    };

}
