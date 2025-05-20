/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderer;

import std;
import vireo;
import lysa.global;
import lysa.configuration;
import lysa.samplers;
import lysa.scene;
import lysa.renderers.renderpass.post_processing;

export namespace lysa {
    class Renderer {
    public:
        struct FrameData {
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            std::shared_ptr<vireo::CommandList>      commandList;
            std::shared_ptr<vireo::RenderTarget>     colorAttachment;
        };

        Renderer(
            const RenderingConfiguration& config,
            const std::wstring& name);

        virtual void resize(const vireo::Extent& extent);

        virtual void update(uint32 frameIndex);

        std::shared_ptr<vireo::Image> getColorAttachment(uint32 frameIndex) const;

        virtual std::vector<std::shared_ptr<const vireo::CommandList>> render(
            uint32 frameIndex,
            Scene& scene);

        virtual void mainColorPass(
            uint32 frameIndex,
            Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::CommandList>& commandList) = 0;

        void addPostprocessing(const std::wstring& fragShaderName, void* data = nullptr, uint32 dataSize = 0);

        void removePostprocessing(const std::wstring& fragShaderName);

        virtual ~Renderer() = default;

    protected:
        const RenderingConfiguration&        config;
        const std::wstring            name;
        const Samplers                samplers;

    private:
        vireo::Extent                                currentExtent{};
        std::vector<FrameData>                       framesData;
        std::vector<std::shared_ptr<PostProcessing>> postProcessingPasses;
    };
}