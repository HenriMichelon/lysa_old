/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.renderer;

namespace lysa {
    Renderer::Renderer(
        const SurfaceConfig& surfaceConfig,
        const std::shared_ptr<vireo::Vireo>& vireo,
        const std::wstring& name) :
        surfaceConfig{surfaceConfig},
        name{name},
        samplers{vireo},
        vireo{vireo},
        submitQueue{vireo->createSubmitQueue(vireo::CommandType::GRAPHIC, name)} {
    }

    void Renderer::waitIdle() const {
        submitQueue->waitIdle();
    }

    void Renderer::addPostprocessing(const std::wstring& fragShaderName, void* data, const uint32_t dataSize) {
        waitIdle();
        const auto postProcessingPass = std::make_shared<PostProcessing>(
            surfaceConfig,
            vireo,
            samplers,
            fragShaderName,
            data,
            dataSize,
            fragShaderName);
        postProcessingPass->resize(currentExtent);
        postProcessingPasses.push_back(postProcessingPass);
    }

    void Renderer::removePostprocessing(const std::wstring& fragShaderName) {
        waitIdle();
        std::erase_if(postProcessingPasses, [&fragShaderName](const std::shared_ptr<PostProcessing>& item) {
            return item->getFragShaderName() == fragShaderName;
        });
    }

}