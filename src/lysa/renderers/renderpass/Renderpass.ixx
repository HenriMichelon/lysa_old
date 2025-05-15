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
import lysa.scene;
import lysa.window_config;
import lysa.renderers.samplers;

export namespace lysa {
    class Renderpass {
    public:
        Renderpass(
            const WindowConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const Samplers& samplers,
            const std::wstring& name);

        virtual void resize(const vireo::Extent& extent) { }

        virtual void update(uint32_t frameIndex) { }

        virtual void render(
            uint32_t frameIndex,
            Scene& scene,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::CommandList>& commandList,
            bool recordLastBarrier = true) = 0;

        virtual ~Renderpass() = default;

    protected:
        const std::wstring                      name;
        const WindowConfig&                    surfaceConfig;
        std::shared_ptr<vireo::Vireo>           vireo;
        const Samplers&                         samplers;
        std::shared_ptr<vireo::GraphicPipeline> pipeline;

    };
}