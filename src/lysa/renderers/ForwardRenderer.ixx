/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.forward_renderer;

import std;
import vireo;
import lysa.scene;
import lysa.window_config;
import lysa.renderers.meshes_renderer;
import lysa.renderers.renderpass.forward_color;

export namespace lysa {
    class ForwardRenderer : public MeshesRenderer {
    public:
        ForwardRenderer(
            const WindowConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::wstring& name);

        void update(uint32_t frameIndex) override;

        void resize(const vireo::Extent& extent) override;

        std::vector<std::shared_ptr<const vireo::CommandList>> render(
            uint32_t frameIndex,
            Scene& scene) override;

        std::shared_ptr<vireo::Image> getColorAttachment(uint32_t frameIndex) const override;

    private:
        struct FrameData {
            std::shared_ptr<vireo::RenderTarget> colorAttachment;
        };

        std::vector<FrameData> framesData;
        ForwardColor           forwardColorPass;
    };
}