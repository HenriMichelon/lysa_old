/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass.post_processing;

import std;
import vireo;
import lysa.global;
import lysa.configuration;
import lysa.samplers;
import lysa.scene;
import lysa.renderers.renderpass;

export namespace lysa {
    class PostProcessing : public Renderpass {
    public:
        PostProcessing(
            const RenderingConfiguration& config,
            const Samplers& samplers,
            const std::wstring& fragShaderName,
            void* data,
            uint32 dataSize,
            const std::wstring& name);

        void update(uint32 frameIndex) override;

        void render(
           uint32 frameIndex,
           const vireo::Viewport&viewport,
           const vireo::Rect&scissor,
           const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
           vireo::CommandList& commandList,
           bool recordLastBarrier);

        void resize(const vireo::Extent& extent) override;

        virtual std::shared_ptr<vireo::Image> getColorAttachment(const uint32 frameIndex) {
            return framesData[frameIndex].colorAttachment->getImage();
        }

        const auto& getFragShaderName() const { return fragShaderName; }

    protected:
        static constexpr vireo::DescriptorIndex BINDING_PARAMS{0};
        static constexpr vireo::DescriptorIndex BINDING_INPUT{1};
        static constexpr vireo::DescriptorIndex BINDING_DATA{2};
        static constexpr vireo::DescriptorIndex BINDING_SAMPLERS{0};

        struct PostProcessingParams {
            uint2  imageSize;
            float  time;
        };

        struct FrameData {
            PostProcessingParams                  params;
            std::shared_ptr<vireo::Buffer>        paramsUniform;
            std::shared_ptr<vireo::DescriptorSet> descriptorSet;
            std::shared_ptr<vireo::RenderTarget>  colorAttachment;
        };

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc = {{}}
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{}}
        };

        const std::wstring                          fragShaderName;
        void*                                       data{nullptr};
        std::shared_ptr<vireo::Buffer>              dataUniform{nullptr};
        std::vector<FrameData>                      framesData;
        std::shared_ptr<vireo::DescriptorLayout>    descriptorLayout;

    };
}