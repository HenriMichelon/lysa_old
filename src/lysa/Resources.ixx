/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.resources_manager;

import vireo;
import lysa.global;
import lysa.configuration;
import lysa.memory;
import lysa.resources.mesh;

export namespace lysa {

    struct VertexData {
        float3 position;
        float3 normal;
        float2 uv;
        float3 tangent;
    };

    class Resources {
    public:
        static constexpr vireo::DescriptorIndex BINDING_VERTEX{0};
        static constexpr vireo::DescriptorIndex BINDING_MATERIAL{1};
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        Resources(const vireo::Vireo& vireo, ResourcesConfiguration& config);

        void waitIdle() const;

        auto& getVertexArray() { return vertexArray; }

        auto& getMaterialArray() { return materialArray; }

        const auto& getDescriptorSet() const { return descriptorSet; }

        void flush();

        void cleanup();

    private:
        const ResourcesConfiguration& config;
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::shared_ptr<vireo::CommandAllocator> commandAllocator;
        std::shared_ptr<vireo::CommandList> commandList;
        MemoryArray vertexArray;
        MemoryArray materialArray;
    };

}