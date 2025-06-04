/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.resources;

import vireo;
import lysa.global;
import lysa.configuration;
import lysa.memory;
import lysa.samplers;
import lysa.resources.image;
import lysa.resources.mesh;

export namespace lysa {

    class Resources {
    public:
        static constexpr auto MAX_TEXTURES{500};

        static constexpr uint32_t SET_RESOURCES{0};
        static constexpr vireo::DescriptorIndex BINDING_VERTEX{0};
        static constexpr vireo::DescriptorIndex BINDING_MATERIAL{1};
        static constexpr vireo::DescriptorIndex BINDING_TEXTURE{2};
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        Resources(
            const vireo::Vireo& vireo,
            ResourcesConfiguration& config,
            const vireo::SubmitQueue& graphicQueue);

        auto& getVertexArray() { return vertexArray; }

        auto& getIndexArray() { return indexArray; }

        auto& getMaterialArray() { return materialArray; }

        auto& getSamplers() { return samplers; }

        uint32 addTexture(const Image& image);

        const auto& getDescriptorSet() const { return descriptorSet; }

        void flush(const vireo::CommandList& commandList);

        void restart();

        void cleanup();

        void setUpdated() { updated = true; }

        auto isUpdated() const { return updated; }

        auto& getMutex() { return mutex; }

        Resources(Resources&) = delete;
        Resources& operator = (Resources&) = delete;

    private:
        const ResourcesConfiguration& config;
        DeviceMemoryArray vertexArray;
        DeviceMemoryArray indexArray;
        DeviceMemoryArray materialArray;
        Samplers samplers;
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        std::vector<std::shared_ptr<vireo::Image>> textures;
        std::shared_ptr<vireo::Image> blankImage;
        bool updated{false};
        bool textureUpdated{false};
        std::mutex mutex;

        static std::vector<uint8> createBlankJPEG();

        static void stb_write_func(void *context, void *data, int size);
    };

}