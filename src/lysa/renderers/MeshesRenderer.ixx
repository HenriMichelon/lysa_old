/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.meshes_renderer;

import std;
import vireo;
import lysa.window_config;
import lysa.renderers.renderer;

export namespace lysa {
    class MeshesRenderer : public Renderer {
    public:
        MeshesRenderer(
            const WindowConfig& surfaceConfig,
            const std::shared_ptr<vireo::Vireo>& vireo,
            const std::wstring& name);

    protected:
        struct FrameData : Renderer::FrameData {};

        std::vector<FrameData> framesData;
    };
}