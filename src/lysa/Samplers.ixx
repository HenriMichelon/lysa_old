/* Copyright (c) 2025-present Henri Michelon
*
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
export module lysa.samplers;

import std;
import vireo;
import lysa.types;

export namespace lysa {

    class Samplers {
    public:
        static constexpr auto MAX_SAMPLERS{20};
        static constexpr uint32 SET_SAMPLERS{1};

        struct SamplerInfo {
            vireo::Filter minFilter;
            vireo::Filter maxFilter;
            vireo::AddressMode samplerAddressModeU;
            vireo::AddressMode samplerAddressModeV;
            float minLod;
            float maxLod;
            bool anisotropyEnable;
            vireo::MipMapMode mipMapMode;
            vireo::CompareOp samplerCompareOp;

            friend bool operator==(const SamplerInfo&l, const SamplerInfo&r) {
                return l.minFilter == r.minFilter &&
                    l.maxFilter == r.maxFilter &&
                    l.samplerAddressModeU == r.samplerAddressModeU &&
                    l.samplerAddressModeV == r.samplerAddressModeV &&
                    l.minLod == r.minLod &&
                    l.maxLod == r.maxLod &&
                    l.anisotropyEnable == r.anisotropyEnable &&
                    l.mipMapMode == r.mipMapMode &&
                    l.samplerCompareOp == r.samplerCompareOp;
            }
        };

        uint32 addSampler(
            vireo::Filter minFilter,
            vireo::Filter maxFilter,
            vireo::AddressMode samplerAddressModeU,
            vireo::AddressMode samplerAddressModeV,
            float minLod = 0.0f,
            float maxLod = vireo::Sampler::LOD_CLAMP_NONE,
            bool anisotropyEnable = true,
            vireo::MipMapMode mipMapMode = vireo::MipMapMode::LINEAR,
            vireo::CompareOp compareOp = vireo::CompareOp::NEVER);

        bool ipUpdated() const { return samplersUpdated; }

        void update();

        const auto& getDescriptorLayout() const { return descriptorLayout; }

        const auto& getDescriptorSet() const { return descriptorSet; }

    private:
        const vireo::Vireo& vireo;
        uint32 samplerCount{0};
        std::vector<std::shared_ptr<vireo::Sampler>> samplers;
        std::vector<SamplerInfo>                     samplersInfo;
        std::shared_ptr<vireo::DescriptorLayout>     descriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>        descriptorSet;
        bool samplersUpdated{false};
        std::mutex mutex;

        friend class Resources;

        Samplers(const vireo::Vireo& vireo);

        void cleanup();

    };

}