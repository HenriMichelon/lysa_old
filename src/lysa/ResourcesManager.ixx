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
import lysa.nodes.camera;
import lysa.nodes.node;
import lysa.renderers.renderer;
import lysa.renderers.scene_data;

export namespace lysa {
    /*
     *
     */
    class ResourcesManager {
    public:
        /**
        *
        */
        ResourcesManager(ResourcesConfiguration& config);

        void waitIdle() const;

        void upload(const std::vector<vireo::BufferUploadInfo>& infos) const;

        void upload(const std::vector<vireo::ImageUploadInfo>& infos) const;

        void upload(MemoryArray& memoryArray) const;

        virtual ~ResourcesManager() = default;

    private:
        const ResourcesConfiguration&         config;


    };

}