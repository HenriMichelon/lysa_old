/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass;

import std;
import vireo;
import lysa.types;
import lysa.configuration;

export namespace lysa {
    /**
     * Base class for a single stage in the renderer frame graph.
     *  - Encapsulate pipelines, descriptor layouts/sets, and render state for
     *    a specific step (e.g., depth pre-pass, G-Buffer, lighting, post FX).
     *  - Provide resize() and update() hooks to recreate resources when the
     *    swap chain changes or per-frame data needs refreshing.
     *
     * Notes:
     *  - Subclasses record their work into command lists provided by Renderer.
     *  - Unless stated otherwise, methods are expected to be called from the
     *    render thread.
     */
    class Renderpass {
    public:
        /**
         * Constructs a render pass with a shared rendering configuration and name.
         * @param config Rendering configuration (frame buffering, formats).
         * @param name   Human-readable name for debugging tools/logs.
         */
        Renderpass(
            const RenderingConfiguration& config,
            const std::string& name);

        /** Recreate pass resources after a resize (default: no-op). */
        virtual void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) { }

        /** Update any per-frame state (default: no-op). */
        virtual void update(uint32 frameIndex) { }

        virtual ~Renderpass() = default;
        Renderpass(Renderpass&) = delete;
        Renderpass& operator=(Renderpass&) = delete;

    protected:
        /** Debug/name label for the pass. */
        const std::string name;
        /** Shared rendering configuration. */
        const RenderingConfiguration& config;

        /** Utility to load a shader module by name (backend-agnostic). */
        std::shared_ptr<vireo::ShaderModule> loadShader(const std::string& shaderName) const;
    };
}