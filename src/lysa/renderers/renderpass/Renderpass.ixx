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
import lysa.samplers;
import lysa.scene;

export namespace lysa {
    class Renderpass {
    public:
        Renderpass(
            const RenderingConfiguration& config,
            const Samplers& samplers,
            const std::wstring& name);

        virtual void resize(const vireo::Extent& extent) { }

        virtual void update(uint32 frameIndex) { }

        virtual ~Renderpass() = default;
        Renderpass(Renderpass&) = delete;
        Renderpass& operator=(Renderpass&) = delete;
    protected:
        const std::wstring                      name;
        const RenderingConfiguration&           config;
        const Samplers&                         samplers;
        std::shared_ptr<vireo::GraphicPipeline> defaultPipeline;

    };
}