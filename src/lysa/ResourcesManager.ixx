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

export namespace lysa {
    /*
     *
     */
    class ResourcesManager {
    public:
        /**
        *
        */
        ResourcesManager(const vireo::Vireo& vireo, ResourcesConfiguration& config);

        void waitIdle() const;


        virtual ~ResourcesManager();

    private:
        const ResourcesConfiguration& config;
        std::shared_ptr<vireo::SubmitQueue> transferQueue;
        MemoryArray vertexArray;
        MemoryArray indexArray;

        void upload(MemoryArray& memoryArray) const;
    };

}