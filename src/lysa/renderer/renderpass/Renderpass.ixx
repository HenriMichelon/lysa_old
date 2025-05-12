/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderpass;

import std;
import vireo;
import lysa.surface_config;

export namespace lysa {
    class Renderpass {
    public:
        Renderpass(const SurfaceConfig& surfaceConfig);

        virtual void render(
            uint32_t frameIndex,
            const vireo::Extent& extent,
            const std::shared_ptr<vireo::RenderTarget>& colorAttachment,
            const std::shared_ptr<vireo::CommandList>& commandList) = 0;

        virtual ~Renderpass() = default;

    private:
        const SurfaceConfig& surfaceConfig;

    };
}