/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass.gamma_correction;


namespace lysa {
    GammaCorrection::GammaCorrection(
        const SurfaceConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const Samplers& samplers):
        PostProcessing{surfaceConfig, vireo, samplers, L"Gamma Correction"} {
        pipelineConfig.fragmentShader = vireo->createShaderModule("shaders/gamma_correction.frag");
        pipeline = vireo->createGraphicPipeline(pipelineConfig);
    }

}