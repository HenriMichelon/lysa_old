/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderer;

namespace lysa {
    Renderer::Renderer(
        const std::shared_ptr<vireo::Vireo>& vireo,
        const std::wstring& name) :
        name{name},
        vireo{vireo} {
        submitQueue = vireo->createSubmitQueue(vireo::CommandType::GRAPHIC);
    }
}