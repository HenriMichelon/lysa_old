/* Copyright (c) 2025-present Henri Michelon
*
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
export module lysa.renderers.samplers;

import std;
import vireo;

export namespace lysa {

    class Samplers {
    public:
        enum class SamplerIndex {
            NEAREST_NEAREST_BORDER_LINEAR = 0,
            LINEAR_LINEAR_EDGE_LINEAR     = 1,
        };

        Samplers(const std::shared_ptr<vireo::Vireo>& vireo);

        const auto& getDescriptorLayout() const { return descriptorLayout; }
        const auto& getDescriptorSet() const { return descriptorSet; }

    private:
        std::vector<std::shared_ptr<vireo::Sampler>> samplers{2};
        std::shared_ptr<vireo::DescriptorLayout>     descriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>        descriptorSet;
    };

}