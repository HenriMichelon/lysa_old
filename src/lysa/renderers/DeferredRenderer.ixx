/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.deferred_renderer;

import std;
import vireo;
import lysa.configuration;
import lysa.scene;
import lysa.resources.material;
import lysa.renderers.renderer;
import lysa.renderers.renderpass.gbuffer_pass;
import lysa.renderers.renderpass.lighting_pass;
import lysa.renderers.renderpass.ssao_pass;
import lysa.renderers.renderpass.post_processing;

export namespace lysa {

    class DeferredRenderer : public Renderer {
    public:
        DeferredRenderer(
            const RenderingConfiguration& config,
            const std::wstring& name);

        void update(uint32 frameIndex) override;

        void updatePipelines(
            const std::unordered_map<pipeline_id,
            std::vector<std::shared_ptr<Material>>>& pipelineIds) override;

        void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) override;

        std::shared_ptr<vireo::RenderTarget> getBloomColorAttachment(const uint32 frameIndex) const override {
            return lightingPass.getBrightnessBuffer(frameIndex);
        }

    protected:
        void colorPass(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex) override;

    private:
        GBufferPass gBufferPass;
        LightingPass lightingPass;
        std::unique_ptr<SSAOPass> ssaoPass;
        std::unique_ptr<PostProcessing> ssaoBlurPass;
    };
}