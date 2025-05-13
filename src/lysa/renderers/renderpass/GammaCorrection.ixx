/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.gamma_correction;

import std;
import vireo;
import lysa.surface_config;
import lysa.renderers.renderpass.post_processing;
import lysa.renderers.samplers;

export namespace lysa {
    class GammaCorrection : public PostProcessing {
    public:
        GammaCorrection(
            const SurfaceConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const Samplers& samplers);
    };
}