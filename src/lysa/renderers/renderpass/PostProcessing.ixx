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
import lysa.scene;
import lysa.window_config;
import lysa.renderers.renderpass;
import lysa.renderers.samplers;

export namespace lysa {
    class PostProcessing : public Renderpass {
    public:
        PostProcessing(
            const WindowConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const Samplers& samplers,
            const std::wstring& fragShaderName,
            void* data,
            uint32_t dataSize,
            const std::wstring& name);

        void update(uint32_t frameIndex) override;

        void render(
           uint32_t frameIndex,
           Scene& scene,
           const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
           const std::shared_ptr<vireo::CommandList>& commandList,
           bool recordLastBarrier) override;

        void resize(const vireo::Extent& extent) override;

        virtual std::shared_ptr<vireo::Image> getColorAttachment(const uint32_t frameIndex) {
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