/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.environment;

import lysa.global;
import lysa.nodes.node;

export namespace lysa {

    /**
     * Environment properties for the scene
     */
    class Environment : public Node {
    public:
        /**
         * Creates en Environment object
         * @param colorAndIntensity Ambient RGB color and intensity
         * @param nodeName Node name.
         */
        Environment(
            float4 colorAndIntensity = {1.0f, 1.0f, 1.0f, 1.0f},
            const std::wstring &nodeName = TypeNames[ENVIRONMENT]);

        ~Environment() override = default;

        /**
         * Returns the ambient RGB color and intensity
         */
        const auto& getAmbientColorAndIntensity() const { return ambientColorIntensity; }

        /**
         * Sets the ambient RGB color and intensity
        */
        auto setAmbientColorAndIntensity(const float4 ambientColorIntensity) {
            this->ambientColorIntensity = ambientColorIntensity;
        }

        void setProperty(const std::string &property, const std::string &value) override;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

    private:
        float4 ambientColorIntensity;
    };

}
