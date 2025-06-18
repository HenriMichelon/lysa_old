/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.renderers.outlines;

import std;
import vireo;
import lysa.math;
import lysa.configuration;
import lysa.renderers.vector;

export namespace lysa {

    class OutlinesRenderer : public VectorRenderer{
    public:
        OutlinesRenderer(const RenderingConfiguration& renderingConfiguration);

    private:
        struct PushConstants {
            float  scale{1.0f};
            float4 color{1.0f, 0.0f, 0.0f, 1.0f};
        };

        PushConstants pushConstants;
    };

}