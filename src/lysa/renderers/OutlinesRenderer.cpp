/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.renderers.outlines;

import lysa.application;
import lysa.nodes.ray_cast;

namespace lysa {

    OutlinesRenderer::OutlinesRenderer(
        const RenderingConfiguration& renderingConfiguration) :
        VectorRenderer{
            true,
            renderingConfiguration,
            L"Outlines Renderer",
            L"outlines",
            {
                .stage = vireo::ShaderStage::VERTEX,
                .size = sizeof(PushConstants),
            },
            &pushConstants} {
    }

}