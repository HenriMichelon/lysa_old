/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.pipelines.frustum_culling;

import vireo;
import lysa.global;

export namespace lysa {
    class FrustumCulling {
    public:

        virtual ~FrustumCulling() = default;
        FrustumCulling(FrustumCulling&) = delete;
        FrustumCulling& operator=(FrustumCulling&) = delete;

    private:
        static constexpr vireo::DescriptorIndex BINDING_GLOBAL{0};
        static constexpr vireo::DescriptorIndex BINDING_MODELS{1};
        static constexpr vireo::DescriptorIndex BINDING_SURFACES{2};
        static constexpr vireo::DescriptorIndex BINDING_OUTPUT{0};

        struct Plane {
            float3 normal;
            float  distance;
        };

        struct Global {
            uint32 pipelineId;
            uint32 surfaceCount;
            Plane  planes[6];
        };

        std::shared_ptr<vireo::CommandAllocator> commandAllocator;
        std::shared_ptr<vireo::CommandList>      commandList;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>    descriptorSet;
        std::shared_ptr<vireo::Buffer>           globalBuffer;
        std::shared_ptr<vireo::Pipeline>         pipeline;
    };
}