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
        const vireo::DescriptorIndex BINDING_GLOBAL{0};
        const vireo::DescriptorIndex BINDING_AABB{1};
        const vireo::DescriptorIndex BINDING_OUTPUT{1};

        struct AABB {
            float3 min;
            float3 max;
        };

        struct Plane {
            float3 normal;
            float  distance;
        };

        struct Global {
            uint objectCount;
            Plane planes[6];
        };

        struct FrameData {
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            std::shared_ptr<vireo::CommandList>      commandList;
            std::shared_ptr<vireo::DescriptorSet>    descriptorSet;
            std::shared_ptr<vireo::Buffer>           globalBuffer;
        };

        virtual ~FrustumCulling() = default;
        FrustumCulling(FrustumCulling&) = delete;
        FrustumCulling& operator=(FrustumCulling&) = delete;

    private:
        std::vector<FrameData> framesData;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
        std::shared_ptr<vireo::Pipeline> pipeline;
    };
}