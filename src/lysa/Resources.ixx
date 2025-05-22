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
        Resources(const vireo::Vireo& vireo, ResourcesConfiguration& config);

        auto& getVertexArray() { return vertexArray; }

        auto& getMaterialArray() { return materialArray; }

        void flush(vireo::CommandList& commandList);

        void cleanup();

        Resources(Resources&) = delete;
        Resources& operator=(Resources&) = delete;

    private:
        const ResourcesConfiguration& config;
        DeviceMemoryArray vertexArray;
        DeviceMemoryArray materialArray;
    };

}