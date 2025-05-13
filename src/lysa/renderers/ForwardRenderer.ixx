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
import lysa.renderers.renderpass.forward_color;

export namespace lysa {
    class ForwardRenderer : public MeshesRenderer {
    public:
        ForwardRenderer(
            const SurfaceConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::wstring& name);

        void update(uint32_t frameIndex) override;

        void resize(const vireo::Extent& extent) override;

        void render(
            uint32_t frameIndex,
            const vireo::Extent& extent,
            const std::shared_ptr<vireo::Semaphore>& renderingFinishedSemaphore) override;

        std::shared_ptr<vireo::Image> getColorAttachment(uint32_t frameIndex) const override;

    private:
        struct FrameData {
            std::shared_ptr<vireo::RenderTarget> colorAttachment;
        };

        std::vector<FrameData> framesData;
        ForwardColor           forwardColorPass;
    };
}