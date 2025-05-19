/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass;

namespace lysa {
    Renderpass::Renderpass(
        const RenderingConfiguration& config,
        const Samplers& samplers,
        const std::wstring& name):
        name{name},
        config{config},
        samplers{samplers} {
    }
}