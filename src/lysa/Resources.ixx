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

    class Resources {
    public:
        static constexpr vireo::DescriptorIndex BINDING_VERTEX{1};
        static constexpr vireo::DescriptorIndex BINDING_INDEX{2};
        static constexpr vireo::DescriptorIndex BINDING_MATERIAL{3};
        static constexpr vireo::DescriptorIndex BINDING_MESH_SURFACE{4};
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        Resources(const vireo::Vireo& vireo, ResourcesConfiguration& config);

        void waitIdle() const;

        auto& getVertexArray() { return vertexArray; }

        auto& getIndexArray() { return indexArray; }

        auto& getMaterialArray() { return materialArray; }

        auto& getMeshSurfaceArray() { return meshSurfaceArray; }

        const auto& getDescriptorSet() const { return descriptorSet; }

        void flush();

        virtual ~Resources();

    private:
        const ResourcesConfiguration& config;
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::shared_ptr<vireo::CommandAllocator> commandAllocator;
        std::shared_ptr<vireo::CommandList> commandList;
        MemoryArray vertexArray;
        MemoryArray indexArray;
        MemoryArray materialArray;
        MemoryArray meshSurfaceArray;

    };

}