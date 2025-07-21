/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.spot_light;

import lysa.global;
import lysa.nodes.light;
import lysa.nodes.omni_light;

export namespace lysa {

    /**
     * %A spotlight, such as a spotlight or a lantern.
     */
    class SpotLight : public OmniLight {
    public:
        /**
         * Creates a SpotLight with default parameters
         */
        SpotLight(const std::string &name = TypeNames[SPOT_LIGHT]);

        /**
        * Create a SpotLight.
        * @param cutOffDegrees the inner cutoff angle that specifies the spotlight's radius, in degrees
        * @param outerCutOffDegrees the outer cutoff angle that specifies the spotlight's radius, in degrees. Everything outside this angle is not lit by the spotlight.
        * @param range Radius of the light and shadows
        * @param color the RGB color and intensity
        * @param nodeName Node name
        */
        SpotLight(float cutOffDegrees,
                  float outerCutOffDegrees,
                  float range,
                  const float4& color = {1.0f, 1.0f, 1.0f, 1.0f},
                  const std::string &nodeName= TypeNames[SPOT_LIGHT]);

        ~SpotLight() override = default;

        /**
         * Sets the inner cutoff angle that specifies the spotlight's radius, in degrees
         */
        void setCutOff(float cutOffDegrees);

        /**
         * Returns the inner cutoff value that specifies the spotlight's radius (not the angle!)
         */
        auto getCutOff() const { return cutOff; }

        /**
         * Sets the outer cutoff angle that specifies the spotlight's radius. Everything outside this angle is not lit by the spotlight.
         */
        void setOuterCutOff(float outerCutOffDegrees);

        /**
         * Returns the outer cutoff value that specifies the spotlight's radius (not the angle!).
         */
        auto getOuterCutOff() const { return outerCutOff; }

        /**
         * Returns the field of view of the spotlight, in radians
         */
        auto getFov() const { return fov; }

        void setProperty(const std::string &property, const std::string &value) override;

        LightData getLightData() const override;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

    private:
        float fov{0.0f};
        float cutOff{std::cos(radians(10.f))};
        float outerCutOff{std::cos(radians(15.f))};

    };

}
