/* Copyright (c) 2025-present Henri Michelon
*
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.samplers;

import lysa.application;
import lysa.global;

namespace lysa {

    Samplers::Samplers(const vireo::Vireo& vireo):
        vireo{vireo},
        samplers(MAX_SAMPLERS),
        samplersInfo(MAX_SAMPLERS) {
        descriptorLayout = vireo.createSamplerDescriptorLayout(L"Static Samplers");
        descriptorLayout->add(0, vireo::DescriptorType::SAMPLER, MAX_SAMPLERS);
        descriptorLayout->build();
        descriptorSet = vireo.createDescriptorSet(descriptorLayout, L"Static Samplers");

        addSampler(vireo::Filter::NEAREST, vireo::Filter::NEAREST, vireo::AddressMode::CLAMP_TO_BORDER, vireo::AddressMode::CLAMP_TO_BORDER);
        addSampler(vireo::Filter::LINEAR, vireo::Filter::LINEAR, vireo::AddressMode::CLAMP_TO_EDGE, vireo::AddressMode::CLAMP_TO_EDGE);
        addSampler(vireo::Filter::LINEAR, vireo::Filter::LINEAR, vireo::AddressMode::REPEAT, vireo::AddressMode::REPEAT);
        for (int i = samplerCount; i < samplers.size(); i++) {
            samplers[i] = samplers[0];
        }
    }

    void Samplers::update() {
        auto lock = std::lock_guard{mutex};
        descriptorSet->update(0, samplers);
        samplersUpdated = false;
    }

    uint32 Samplers::addSampler(
           const vireo::Filter minFilter,
           const vireo::Filter maxFilter,
           const vireo::AddressMode samplerAddressModeU,
           const vireo::AddressMode samplerAddressModeV) {
        auto lock = std::lock_guard{mutex};
        if (samplerCount >= MAX_SAMPLERS) {
            throw Exception("Too many samplers");
        }
        const auto samplerInfo = SamplerInfo{minFilter, maxFilter, samplerAddressModeU, samplerAddressModeV};
        for (int i = 0; i < samplerCount; i++) {
            if (samplersInfo[i] == samplerInfo) {
                return i;
            }
        }
        const auto index = samplerCount;
        samplers[index] = vireo.createSampler(
            minFilter,
            maxFilter,
            samplerAddressModeU,
            samplerAddressModeV,
            samplerAddressModeV,
            0.0f,
            vireo::Sampler::LOD_CLAMP_NONE,
            true,
            vireo::MipMapMode::LINEAR);
        samplersInfo[index] = samplerInfo;
        samplersUpdated = true;
        samplerCount++;
        return index;
    }

    void Samplers::cleanup() {
        samplers.clear();
        descriptorSet.reset();
        descriptorLayout.reset();
    }

}