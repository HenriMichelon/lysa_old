/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderer;

import std;
import vireo;
import lysa.surface_config;
import lysa.renderers.samplers;
import lysa.renderers.renderpass.post_processing;

export namespace lysa {
    class Renderer {
    public:
        struct FrameDataCommand {
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            std::shared_ptr<vireo::CommandList>      commandList;
        };

        Renderer(
            const SurfaceConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::wstring& name);

        virtual void resize(const vireo::Extent& extent) { currentExtent = extent; }

        virtual void update(uint32_t frameIndex) { }

        virtual std::shared_ptr<vireo::Image> getColorAttachment(uint32_t frameIndex) const = 0;

        virtual std::vector<std::shared_ptr<const vireo::CommandList>> render(
            uint32_t frameIndex,
            const vireo::Extent& extent) = 0;

        void addPostprocessing(const std::wstring& fragShaderName, void* data = nullptr, uint32_t dataSize = 0);

        void removePostprocessing(const std::wstring& fragShaderName);

        virtual ~Renderer() = default;

    protected:
        const SurfaceConfig&                surfaceConfig;
        const std::wstring                  name;
        const Samplers                      samplers;
        std::shared_ptr<vireo::Vireo>       vireo;
        vireo::Extent                       currentExtent;
        std::vector<std::shared_ptr<PostProcessing>> postProcessingPasses;
    };
}