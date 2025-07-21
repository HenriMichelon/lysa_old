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
    class Renderpass {
    public:
        Renderpass(
            const RenderingConfiguration& config,
            const std::string& name);

        virtual void resize(const vireo::Extent& extent, const std::shared_ptr<vireo::CommandList>& commandList) { }

        virtual void update(uint32 frameIndex) { }

        virtual ~Renderpass() = default;
        Renderpass(Renderpass&) = delete;
        Renderpass& operator=(Renderpass&) = delete;

    protected:
        const std::string name;
        const RenderingConfiguration& config;

        std::shared_ptr<vireo::ShaderModule> loadShader(const std::string& shaderName) const;
    };
}