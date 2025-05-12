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

export namespace lysa {
    class Renderer {
    public:
        struct FrameDataCommand {
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            std::shared_ptr<vireo::CommandList>      commandList;
        };

        Renderer(const SurfaceConfig& surfaceConfig, const std::shared_ptr<vireo::Vireo>& vireo, const std::wstring& name);

        virtual void resize(const vireo::Extent& extent) { }

        virtual void update(uint32_t frameIndex) { }

        virtual std::shared_ptr<vireo::Image> getColorAttachment(uint32_t frameIndex) = 0;

        virtual void render(
            uint32_t frameIndex,
            const vireo::Extent& extent,
            const std::shared_ptr<vireo::Semaphore>& renderingFinishedSemaphore) { }

        virtual ~Renderer() = default;

    protected:
        const SurfaceConfig& surfaceConfig;
        const std::wstring name;
        std::shared_ptr<vireo::Vireo> vireo;
        std::shared_ptr<vireo::SubmitQueue> submitQueue;
    };
}