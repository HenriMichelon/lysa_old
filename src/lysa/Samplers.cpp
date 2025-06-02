/* Copyright (c) 2025-present Henri Michelon
*
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
module lysa.samplers;

import lysa.application;

namespace lysa {

    Samplers::Samplers() {
        samplers[static_cast<int>(SamplerIndex::NEAREST_NEAREST_BORDER_LINEAR)] = Application::getVireo().createSampler(
            vireo::Filter::NEAREST,
            vireo::Filter::NEAREST,
            vireo::AddressMode::CLAMP_TO_BORDER,
            vireo::AddressMode::CLAMP_TO_BORDER,
            vireo::AddressMode::CLAMP_TO_BORDER,
            0.0f,
            vireo::Sampler::LOD_CLAMP_NONE,
            true,
            vireo::MipMapMode::LINEAR);
        samplers[static_cast<int>(SamplerIndex::LINEAR_LINEAR_EDGE_LINEAR)] = Application::getVireo().createSampler(
            vireo::Filter::LINEAR,
            vireo::Filter::LINEAR,
            vireo::AddressMode::CLAMP_TO_EDGE,
            vireo::AddressMode::CLAMP_TO_EDGE,
            vireo::AddressMode::CLAMP_TO_EDGE,
            0.0f,
            vireo::Sampler::LOD_CLAMP_NONE,
            true,
            vireo::MipMapMode::LINEAR);
        samplers[static_cast<int>(SamplerIndex::LINEAR_LINEAR_REPEAT_LINEAR)] = Application::getVireo().createSampler(
            vireo::Filter::LINEAR,
            vireo::Filter::LINEAR,
            vireo::AddressMode::REPEAT,
            vireo::AddressMode::REPEAT,
            vireo::AddressMode::REPEAT,
            0.0f,
            vireo::Sampler::LOD_CLAMP_NONE,
            true,
            vireo::MipMapMode::LINEAR);

        descriptorLayout = Application::getVireo().createSamplerDescriptorLayout(L"Static Samplers");
        for (int i = 0; i < samplers.size(); i++) {
            descriptorLayout->add(i, vireo::DescriptorType::SAMPLER);
        }
        descriptorLayout->build();

        descriptorSet = Application::getVireo().createDescriptorSet(descriptorLayout, L"Static Samplers");
        for (int i = 0; i < samplers.size(); i++) {
            descriptorSet->update(i, samplers[i]);
        }
    }

}