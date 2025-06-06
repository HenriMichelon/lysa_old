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
            pipeline_id pipelineId,
            uint32 surfaceCount,
            const Camera& camera,
            const vireo::Buffer& outputSurfaces,
            const vireo::Buffer& outputSurfacesCounter,
            const vireo::Buffer& outputIndices,
            const vireo::Buffer& command) ;

        virtual ~FrustumCulling() = default;
        FrustumCulling(FrustumCulling&) = delete;
        FrustumCulling& operator=(FrustumCulling&) = delete;

    private:
        static constexpr vireo::DescriptorIndex BINDING_CULLING_GLOBAL{0};
        static constexpr vireo::DescriptorIndex BINDING_CULLING_MODELS{1};
        static constexpr vireo::DescriptorIndex BINDING_CULLING_MATERIALS{2};
        static constexpr vireo::DescriptorIndex BINDING_CULLING_SURFACE{3};
        static constexpr vireo::DescriptorIndex BINDING_CULLING_OUTPUT{4};
        static constexpr vireo::DescriptorIndex BINDING_CULLING_COUNTER{5};

        static constexpr vireo::DescriptorIndex BINDING_COMMAND_INDICES{0};
        static constexpr vireo::DescriptorIndex BINDING_COMMAND_SURFACES{1};
        static constexpr vireo::DescriptorIndex BINDING_COMMAND_INPUT{2};
        static constexpr vireo::DescriptorIndex BINDING_COMMAND_COUNTER{3};
        static constexpr vireo::DescriptorIndex BINDING_COMMAND_OUTPUT{4};
        static constexpr vireo::DescriptorIndex BINDING_COMMAND_COMMAND{5};

        const std::wstring DEBUG_NAME{L"FrustumCulling"};

        struct Global {
            uint32 pipelineId;
            uint32 surfaceCount;
            Frustum::Plane planes[6];
        };

        std::shared_ptr<vireo::DescriptorLayout> cullingDescriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>    cullingDescriptorSet;
        std::shared_ptr<vireo::DescriptorLayout> commandDescriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>    commandDescriptorSet;
        std::shared_ptr<vireo::Buffer>           globalBuffer;
        std::shared_ptr<vireo::Buffer>           commandClearBuffer;
        std::shared_ptr<vireo::Pipeline>         pipelineCulling;
        std::shared_ptr<vireo::Pipeline>         pipelineDrawCommand;
    };
}