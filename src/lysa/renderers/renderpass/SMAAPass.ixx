/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.smaa_pass;

import std;
import vireo;
import lysa.configuration;
import lysa.math;
import lysa.renderers.renderpass;
import lysa.renderers.renderpass.post_processing;

export namespace lysa {

    class SMAAPass : public Renderpass {
    public:
        SMAAPass(const RenderingConfiguration& config);

        void render(
            vireo::CommandList& commandList,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            uint32 frameIndex);

        void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) override;

        virtual std::shared_ptr<vireo::RenderTarget> getColorAttachment(const uint32 frameIndex) {
            return framesData[frameIndex].colorBuffer;
        }

    private:
        const std::wstring EDGE_DETECT_FRAGMENT_SHADER{L"smaa_edge_detect.frag"};
        const std::wstring BLEND_WEIGHT_FRAGMENT_SHADER{L"smaa_blend_weight.frag"};
        const std::wstring BLEND_FRAGMENT_SHADER{L"smaa_neighborhood_blend.frag"};

        struct Data {
            float edgeThreshold;
            int   blendMaxSteps;
        };

        struct FrameData {
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;
            std::shared_ptr<vireo::RenderTarget>  edgeDetectBuffer;
            std::shared_ptr<vireo::RenderTarget>  blendWeightBuffer;
            std::shared_ptr<vireo::RenderTarget>  colorBuffer;
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorRenderFormats = { vireo::ImageFormat::R16G16_SFLOAT },
            .colorBlendDesc = {{}},
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{ }},
        };

        Data data;
        std::vector<FrameData> framesData;
        PostProcessing::PostProcessingParams params;
        std::shared_ptr<vireo::Buffer> dataBuffer;
        std::shared_ptr<vireo::Buffer> paramsBuffer;
        std::vector<std::shared_ptr<vireo::Image>> textures;
        std::shared_ptr<vireo::GraphicPipeline> edgeDetectPipeline;
        std::shared_ptr<vireo::GraphicPipeline> blendWeightPipeline;
        std::shared_ptr<vireo::GraphicPipeline> blendPipeline;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
    };
}