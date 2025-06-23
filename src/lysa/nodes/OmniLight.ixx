/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.omni_light;

import lysa.nodes.light;

export namespace lysa {

    /**
     * Omnidirectional light, such as a light bulb or a candle
     */
    class OmniLight : public Light {
    public:
        /**
         * Creates an OmniLight with default parameters
         */
        OmniLight(const std::wstring &name = TypeNames[OMNI_LIGHT], Type type = OMNI_LIGHT);

        /**
         * Create an OmniLight.
         * @param range The light's radius
         * @param color the RGB color and intensity
         * @param nodeName Node name
         * @param type Omni or Spot light
         */
        OmniLight(float range,
                  const float4& color = {1.0f, 1.0f, 1.0f, 1.0f},
                  const std::wstring &nodeName =TypeNames[OMNI_LIGHT],
                  Type type = OMNI_LIGHT);

        ~OmniLight() override = default;

        /**
         * Returns the light range (default 10m)
         */
        auto getRange() const { return range; }

        /**
         * Sets the light range
         */
        void setRange(float range);

        /**
         * Returns the light near clipping distance for the shadows (default 0.1m)
         */
        auto getNearClipDistance() const { return near; }

        void setProperty(const std::string &property, const std::string &value) override;

        LightData getLightData() const override;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

    private:
        // Maximum distance of lighting
        float range{10.0f};
        // clipping distance for shadows
        float near{0.1f};
    };

}
