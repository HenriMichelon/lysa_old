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
    /**
     * Forward rendering path.
     *
     * Renders opaque geometry directly to the color/depth attachments using a
     * single forward pass, then handles transparency and optional bloom.
     * Suitable for scenes with many materials requiring complex shading (PBR,
     * alpha test, etc.) without a G-Buffer.
     */
    class ForwardRenderer : public Renderer {
    public:
        /**
         * Constructs a forward renderer instance.
         * @param config Rendering configuration (attachments, frames in flight).
         * @param name   Human-readable name for debugging.
         */
        ForwardRenderer(
            const RenderingConfiguration& config,
            const std::string& name);

        /** Recreates attachments/pipelines after a resize. */
        void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) override;

        /** Updates/creates pipelines following the materials mapping. */
        void updatePipelines(
            const std::unordered_map<pipeline_id,
            std::vector<std::shared_ptr<Material>>>& pipelineIds) override;

        /** Returns the brightness buffer used for bloom extraction. */
        std::shared_ptr<vireo::RenderTarget> getBloomColorAttachment(const uint32 frameIndex) const override {
            return forwardColorPass.getBrightnessBuffer(frameIndex);
        }

    protected:
        /** Per-frame housekeeping (post-process data, etc.). */
        void update(uint32 frameIndex) override;

        /** Records the forward color pass followed by transparency. */
        void colorPass(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex) override;

    private:
        /** Opaque/alpha-tested color pass used by forward rendering. */
        ForwardColor forwardColorPass;
    };
}