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
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderer;
import lysa.renderers.renderpass.forward_color;
import lysa.renderers.renderpass.transparency_pass;

export namespace lysa {
    class ForwardRenderer : public Renderer {
    public:
        ForwardRenderer(
            const RenderingConfiguration& config,
            const std::wstring& name);

        void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) override;

        void updatePipelines(
            const std::unordered_map<pipeline_id,
            std::vector<std::shared_ptr<Material>>>& pipelineIds) override;

    protected:
        void update(uint32 frameIndex) override;

        void colorPass(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex) override;

    private:
        ForwardColor forwardColorPass;
    };
}