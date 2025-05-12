/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.renderer;

import std;
import vireo;

export namespace lysa {
    class Renderer {
    public:
        Renderer(const std::shared_ptr<vireo::Vireo>& vireo, const std::wstring& name);

        virtual void resize() { }

        virtual void update(uint32_t frameIndex) { }

        virtual void render(uint32_t frameIndex) { }

        virtual ~Renderer() = default;

    protected:
        const std::wstring name;
        std::shared_ptr<vireo::Vireo> vireo;
        std::shared_ptr<vireo::SubmitQueue> submitQueue;
    };
}