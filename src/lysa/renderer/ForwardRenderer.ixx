/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.forward_renderer;

import std;
import vireo;
import lysa.surface_config;
import lysa.renderers.meshes_renderer;

export namespace lysa {
    class ForwardRenderer : public MeshesRenderer {
    public:
        ForwardRenderer(const SurfaceConfig& surfaceConfig, const std::shared_ptr<vireo::Vireo>& vireo, const std::wstring& name);

        void resize(const vireo::Extent& extent) override;

        void render(
            uint32_t frameIndex,
            const vireo::Extent& extent,
            const std::shared_ptr<vireo::Semaphore>& renderingFinishedSemaphore) override;

        std::shared_ptr<vireo::Image> getColorAttachment(const uint32_t frameIndex) override {
            return framesData[frameIndex].colorAttachment->getImage();
        }

    private:
        struct FrameData {
            std::shared_ptr<vireo::RenderTarget> colorAttachment;
        };
        std::vector<FrameData> framesData;

        vireo::GraphicPipelineConfiguration pipelineConfig {
            .colorBlendDesc   = { { .blendEnable = true } },
            .depthTestEnable  = false,
            .depthWriteEnable = false,
        };

        vireo::RenderingConfiguration renderingConfig {
            .colorRenderTargets = {{
                .clear = true,
            }},
            .depthTestEnable = pipelineConfig.depthTestEnable,
            .discardDepthStencilAfterRender = true,
        };
    };
}