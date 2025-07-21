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
            bool isForScene,
            const DeviceMemoryArray& meshInstancesArray);

        void dispatch(
            vireo::CommandList& commandList,
            uint32 drawCommandsCount,
            const float4x4& view,
            const float4x4& projection,
            const vireo::Buffer& instances,
            const vireo::Buffer& input,
            const vireo::Buffer& output,
            const vireo::Buffer& counter);

        uint32 getDrawCommandsCount() const;

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

        const std::string DEBUG_NAME{"FrustumCulling"};
        const std::string SHADER_SCENE{"frustum_culling.comp"};
        const std::string SHADER_SHADOWMAP{"frustum_culling_shadowmap.comp"};

        struct Global {
            uint32 drawCommandsCount;
            Frustum::Plane planes[6];
            float4x4 viewMatrix;
        };

        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>    descriptorSet;
        std::shared_ptr<vireo::Buffer>           globalBuffer;
        std::shared_ptr<vireo::Buffer>           commandClearCounterBuffer;
        std::shared_ptr<vireo::Buffer>           downloadCounterBuffer;
        std::shared_ptr<vireo::Pipeline>         pipeline;
    };
}