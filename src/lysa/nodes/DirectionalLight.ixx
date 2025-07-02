/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.directional_light;

import lysa.nodes.light;

export namespace lysa {

    /**
     * Directional light from a distance, as from the Sun.
     */
    class DirectionalLight : public Light {
    public:
        static constexpr uint32 MAX_SHADOW_MAP_CASCADES{3};

        /**
         * Create a DirectionalLight
         * @param color RGB color and intensity of the light
         * @param nodeName Node name
         */
        DirectionalLight(const float4& color = {1.0f, 1.0f, 1.0f, 1.0f},
                                  const std::wstring &nodeName = TypeNames[DIRECTIONAL_LIGHT]);

        ~DirectionalLight() override = default;

        /**
         * Sets the number of cascades for the shadow map (between 2 and ShadowMapFrameBuffer::CASCADED_SHADOWMAP_MAX_LAYERS).<br>
         * *must* be called before adding the light to the scene since this value is used when instancing the shadow map
         * renderer for this light.
         */
        void setShadowMapCascadesCount(uint32 cascadesCount);

        /**
         * Returns the number of cascades for the shadow map
         */
        auto getShadowMapCascadesCount() const { return shadowMapCascadesCount; }

        LightData getLightData() const override;

        void setProperty(const std::string &property, const std::string &value) override;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

    private:
        uint32 shadowMapCascadesCount{3};
    };

}
