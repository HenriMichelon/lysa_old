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
            const DeviceMemoryArray& modelsArray,
            const DeviceMemoryArray& surfacesArray);

        void dispatch(
            vireo::CommandList& commandList,
            float aspectRatio,
            pipeline_id pipelineId,
            uint32 indexCount,
            const Camera& camera,
            const vireo::Buffer& input,
            const vireo::Buffer& output) ;

        uint32 getCounter() const;

        virtual ~FrustumCulling() = default;
        FrustumCulling(FrustumCulling&) = delete;
        FrustumCulling& operator=(FrustumCulling&) = delete;

    private:
        static constexpr vireo::DescriptorIndex BINDING_GLOBAL{0};
        static constexpr vireo::DescriptorIndex BINDING_MODELS{1};
        static constexpr vireo::DescriptorIndex BINDING_MATERIALS{2};
        static constexpr vireo::DescriptorIndex BINDING_SURFACES{3};
        static constexpr vireo::DescriptorIndex BINDING_INPUT{4};
        static constexpr vireo::DescriptorIndex BINDING_OUTPUT{5};
        static constexpr vireo::DescriptorIndex BINDING_COUNTER{6};
        const std::wstring DEBUG_NAME{L"FrustumCulling"};

        struct Global {
            uint32 pipelineId;
            uint32 indexCount;
            Frustum::Plane planes[6];
        };

        // std::shared_ptr<vireo::CommandAllocator> commandAllocator;
        // std::shared_ptr<vireo::CommandList>      commandList;
        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>    descriptorSet;
        std::shared_ptr<vireo::Buffer>           globalBuffer;
        std::shared_ptr<vireo::Buffer>           counterBuffer;
        std::shared_ptr<vireo::Pipeline>         pipeline;
    };
}