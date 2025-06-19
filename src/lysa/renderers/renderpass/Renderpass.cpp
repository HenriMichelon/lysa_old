/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderpass;

import lysa.application;
import lysa.virtual_fs;

namespace lysa {

    Renderpass::Renderpass(
        const RenderingConfiguration& config,
        const std::wstring& name):
        name{name},
        config{config} {
    }

    std::shared_ptr<vireo::ShaderModule> Renderpass::loadShader(const std::wstring& shaderName) const {
        const auto& vireo = Application::getVireo();
        auto tempBuffer = std::vector<char>{};
        const auto& ext = vireo.getShaderFileExtension();
        VirtualFS::loadBinaryData(L"app://" + Application::getConfiguration().shaderDir + L"/" + shaderName + ext, tempBuffer);
        return vireo.createShaderModule(tempBuffer);
    }
}