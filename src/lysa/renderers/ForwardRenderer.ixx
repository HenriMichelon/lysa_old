/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.forward_renderer;

import std;
import vireo;
import lysa.configuration;
import lysa.renderers.renderer;
import lysa.renderers.scene_data;
import lysa.renderers.renderpass.forward_color;

export namespace lysa {
    class ForwardRenderer : public Renderer {
    public:
        ForwardRenderer(
            const RenderingConfiguration& config,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::wstring& name);

        void update(uint32 frameIndex) override;

        void resize(const vireo::Extent& extent) override;

        void mainColorPass(
            uint32 frameIndex,
            SceneData& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::CommandList>& commandList) override;

    private:
        ForwardColor forwardColorPass;
    };
}