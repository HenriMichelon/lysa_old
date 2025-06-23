/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.pipelines.frustum_culling;

import vireo;
import lysa.frustum;
import lysa.global;
import lysa.memory;
import lysa.nodes.camera;

export namespace lysa {
    class FrustumCulling {
    public:
        FrustumCulling(
            const DeviceMemoryArray& meshInstancesArray);

        void dispatch(
            vireo::CommandList& commandList,
            uint32 drawCommandsCount,
            const Camera& camera,
            const vireo::Buffer& instances,
            const vireo::Buffer& input,
            const vireo::Buffer& output,
            const vireo::Buffer& counter) ;

        virtual ~FrustumCulling() = default;
        FrustumCulling(FrustumCulling&) = delete;
        FrustumCulling& operator=(FrustumCulling&) = delete;

    private:
        static constexpr vireo::DescriptorIndex BINDING_GLOBAL{0};
        static constexpr vireo::DescriptorIndex BINDING_MESHINSTANCES{1};
        static constexpr vireo::DescriptorIndex BINDING_INSTANCES{2};
        static constexpr vireo::DescriptorIndex BINDING_INPUT{3};
        static constexpr vireo::DescriptorIndex BINDING_OUTPUT{4};
        static constexpr vireo::DescriptorIndex BINDING_COUNTER{5};

        const std::wstring DEBUG_NAME{L"FrustumCulling"};

        struct Global {
            uint32 drawCommandsCount;
            Frustum::Plane planes[6];
            float4x4 viewMatrix;
        };

        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>    descriptorSet;
        std::shared_ptr<vireo::Buffer>           globalBuffer;
        std::shared_ptr<vireo::Buffer>           commandClearCounterBuffer;
        std::shared_ptr<vireo::Pipeline>         pipeline;
    };
}