/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderer;

import vireo;
import lysa.configuration;
import lysa.math;
import lysa.samplers;
import lysa.scene;
import lysa.types;
import lysa.resources.material;
import lysa.renderers.renderpass.post_processing;
import lysa.renderers.renderpass.depth_prepass;
import lysa.renderers.renderpass.shader_material_pass;
import lysa.renderers.renderpass.shadow_map_pass;
import lysa.renderers.renderpass.smaa_pass;
import lysa.renderers.renderpass.transparency_pass;

export namespace lysa {
    /**
     * High-level scene renderer orchestrating the frame graph.
     *  - Own and manage the set of render passes required by a rendering path
     *    (depth pre-pass, opaque/transparent color, shader-material passes, SMAA,
     *    bloom and other post-processing).
     *  - Allocate per-frame color/depth attachments and expose them to callers.
     *  - Update and (re)build graphics pipelines when the set of materials changes.
     *  - Drive the typical frame flow: compute → preRender → render → postprocess.
     *
     * Notes:
     *  - Instances are created by Window according to the selected rendering mode
     *    (ForwardRenderer, DeferredRenderer). Thread-safety is not guaranteed;
     *    call methods from the render thread.
     */
    class Renderer {
    public:
        /** Per-frame attachments owned by the renderer. */
        struct FrameData {
            std::shared_ptr<vireo::RenderTarget> colorAttachment;
            std::shared_ptr<vireo::RenderTarget> depthAttachment;
        };

        /**
         * Constructs a renderer with the given configuration.
         * @param config      Global rendering configuration (formats, buffering).
         * @param withStencil True when depth buffers include a stencil component.
         * @param name        Human-readable renderer name (for debugging).
         */
        Renderer(
            const RenderingConfiguration& config,
            bool withStencil,
            const std::string& name);

        /**
         * Recreates attachments/pipelines after a resize.
         * @param extent       New swap chain extent.
         * @param commandList  Command list used for any required transitions/copies.
         */
        virtual void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList);

        /** Returns the color attachment of the current renderer for the frame. */
        std::shared_ptr<vireo::RenderTarget> getColorAttachment(uint32 frameIndex) const;

        /** Short-hand accessor for the color render target of the frame. */
        auto getColorRenderTarget(const uint32 frameIndex) const {
            return framesData[frameIndex].colorAttachment;
        }

        /** Short-hand accessor for the depth render target of the frame. */
        auto getDepthRenderTarget(const uint32 frameIndex) const {
            return framesData[frameIndex].depthAttachment;
        }

        /**
         * Returns the color buffer used for bloom extraction for the frame.
         * Concrete renderers decide which buffer carries the bright pass.
         */
        virtual std::shared_ptr<vireo::RenderTarget> getBloomColorAttachment(uint32 frameIndex) const = 0;

        /**
         * Updates graphics pipelines according to the Scene material mapping.
         * Convenience overload that pulls mapping from the Scene.
         */
        void updatePipelines(const Scene& scene);

        /**
         * Updates graphics pipelines according to the provided materials mapping.
         * @param pipelineIds Map of pipeline family id to materials.
         */
        virtual void updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds);

        /** Performs per-frame housekeeping (e.g., pass-local data updates). */
        virtual void update(uint32 frameIndex);

        /** Executes compute workloads such as frustum culling. */
        void compute(
            vireo::CommandList& commandList,
            Scene& scene,
            uint32 frameIndex) const;

        /** Pre-render stage: uploads, layout transitions, and shadow maps. */
        void preRender(
            vireo::CommandList& commandList,
            const Scene& scene,
            uint32 frameIndex);

        /** Main render stage: records opaque/transparent draw calls. */
        void render(
            vireo::CommandList& commandList,
            const Scene& scene,
            bool clearAttachment,
            uint32 frameIndex);

        /** Applies post-processing chain (SMAA, bloom, custom passes). */
        void postprocess(
            vireo::CommandList& commandList,
            const vireo::Viewport&viewport,
            const vireo::Rect&scissor,
            uint32 frameIndex);

        /** Adds a full-screen post-processing pass by fragment shader name. */
        void addPostprocessing(
            const std::string& fragShaderName,
            vireo::ImageFormat outputFormat,
            void* data = nullptr,
            uint32 dataSize = 0);

        /** Removes a previously added post-processing pass by fragment name. */
        void removePostprocessing(const std::string& fragShaderName);

        virtual ~Renderer() = default;
        Renderer(Renderer&) = delete;
        Renderer& operator=(Renderer&) = delete;

    protected:
        /** Constant buffer data used by Gaussian blur post-process. */
        struct BlurData {
            uint32 kernelSize;
            float4 weights[9*9]; // float4 for correct alignment
            float2 texelSize;
        };

        const RenderingConfiguration& config;
        const std::string name;
        const bool withStencil;

        /**
         * Records the pipeline-specific color pass for the concrete renderer.
         * Implementations dispatch scene draws into colorAttachment/depthAttachment.
         */
        virtual void colorPass(
            vireo::CommandList& commandList,
            const Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::RenderTarget>& depthAttachment,
            bool clearAttachment,
            uint32 frameIndex) = 0;

        /** Precomputes Gaussian weights and texel size based on extent/strength. */
        void updateBlurData(BlurData& blurData, const vireo::Extent& extent, float strength) const;

    private:
        /** Gamma/exposure parameters used by the tone-mapping/post chain. */
        struct {
            float gamma;
            float exposure;
        } gammaCorrectionData;

        /** Default FXAA configuration used when SMAA is disabled. */
         struct {
            float spanMax{8.0f};
            float reduceMul{1.0f / 8.0f};
            float reduceMin{1.0f / 128.0f};
        } fxaaData;

        vireo::Extent          currentExtent{};
        std::vector<FrameData> framesData;
        /** Depth-only pre-pass used by both forward and deferred paths. */
        DepthPrepass           depthPrePass;
        /** Optional pass that renders objects using custom shader materials. */
        ShaderMaterialPass     shaderMaterialPass;
        /** Transparent objects pass (sorted/blended). */
        TransparencyPass       transparencyPass;

        BlurData bloomBlurData;
        std::unique_ptr<SMAAPass> smaaPass;
        std::unique_ptr<PostProcessing> bloomBlurPass;
        /** List of active post-processing passes applied after color pass. */
        std::vector<std::shared_ptr<PostProcessing>> postProcessingPasses;
    };
}