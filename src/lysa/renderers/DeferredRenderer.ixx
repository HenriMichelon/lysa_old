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

    /**
     * Deferred rendering path.
     *
     * Renders geometry into a G-Buffer (position, normal, albedo, emissive),
     * then performs lighting as a full-screen pass, followed by transparency
     * and optional SSAO and bloom post-processing.
     */
    class DeferredRenderer : public Renderer {
    public:
        /**
         * Constructs a deferred renderer instance.
         * @param config Rendering configuration (attachments, frames in flight).
         * @param name   Human-readable name for debugging.
         */
        DeferredRenderer(
            const RenderingConfiguration& config,
            const std::string& name);

        /** Performs per-frame housekeeping (e.g., SSAO/bloom data updates). */
        void update(uint32 frameIndex) override;

        /** Updates/creates pipelines following the materials mapping. */
        void updatePipelines(
            const std::unordered_map<pipeline_id,
            std::vector<std::shared_ptr<Material>>>& pipelineIds) override;

        /** Recreates attachments/pipelines after a resize. */
        void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) override;

        /** Returns the brightness buffer used for bloom extraction. */
        std::shared_ptr<vireo::RenderTarget> getBloomColorAttachment(const uint32 frameIndex) const override {
            return lightingPass.getBrightnessBuffer(frameIndex);
        }

    protected:
        /** Records G-Buffer population then lighting resolve into color attachment. */
        void colorPass(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex) override;

    private:
        /** Constant data used by SSAO blur post-process. */
        BlurData ssaoBlurData;
        /** G-Buffer generation pass. */
        GBufferPass gBufferPass;
        /** Lighting resolve pass consuming the G-Buffer. */
        LightingPass lightingPass;
        /** Optional SSAO pass (created based on configuration). */
        std::unique_ptr<SSAOPass> ssaoPass;
        /** Optional blur pass applied to the SSAO result. */
        std::unique_ptr<PostProcessing> ssaoBlurPass;
    };
}