/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.meshes_renderer;

namespace lysa {
    MeshesRenderer::MeshesRenderer(
        const WindowConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const std::wstring& name) :
        Renderer{surfaceConfig, vireo, name} {
        framesData.resize(surfaceConfig.framesInFlight);
        for (auto& frame : framesData) {
            frame.commandAllocator = vireo->createCommandAllocator(vireo::CommandType::GRAPHIC);
            frame.commandList = frame.commandAllocator->createCommandList();
        }
    }
}