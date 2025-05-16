/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass;

import std;
import vireo;
import lysa.global;
import lysa.configuration;
import lysa.renderers.samplers;
import lysa.renderers.scene_data;

export namespace lysa {
    class Renderpass {
    public:
        Renderpass(
            const RenderingConfiguration& config,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const Samplers& samplers,
            const std::wstring& name);

        virtual void resize(const vireo::Extent& extent) { }

        virtual void update(uint32 frameIndex) { }

        virtual void render(
            uint32 frameIndex,
            SceneData& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::CommandList>& commandList,
            bool recordLastBarrier = true) = 0;

        virtual ~Renderpass() = default;

    protected:
        const std::wstring                      name;
        const RenderingConfiguration&                  config;
        std::shared_ptr<vireo::Vireo>           vireo;
        const Samplers&                         samplers;
        std::shared_ptr<vireo::GraphicPipeline> pipeline;

    };
}