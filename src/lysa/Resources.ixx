/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.resources;

import vireo;
import lysa.configuration;
import lysa.memory;
import lysa.samplers;
import lysa.types;
import lysa.resources.image;
import lysa.resources.mesh;

export namespace lysa {

    /**
     * Central container responsible for GPU-ready shared resources.
     *
     * The Resources class owns and manages device memory arrays (vertices, indices,
     * materials, mesh surfaces), the global sampler set, the descriptor layout/set
     * used by draw pipelines, and texture images including fallback (blank) assets.
     *  - Allocate and provide access to large device-side arrays used by the renderers.
     *  - Maintain a descriptor set exposing materials, surfaces and textures to shaders.
     *  - Manage a pool of sampled images with a fixed upper bound (MAX_TEXTURES).
     *  - Provide default blank 2D and cube textures to avoid null bindings.
     *  - Track and expose an "updated" flag so dependent systems can react.
     *
     * Thread-safety:
     *  - Public methods that mutate internal state should be guarded by the provided mutex.
     *  - Read-only getters may be called concurrently once initialization is complete.
     */
    class Resources {
    public:
        /** Maximum number of textures managed and bound by this container. */
        static constexpr auto MAX_TEXTURES{500};

        /** Descriptor set index used by pipelines to bind shared resources. */
        static constexpr uint32 SET_RESOURCES{0};
        /** Descriptor binding index for the material buffer. */
        static constexpr vireo::DescriptorIndex BINDING_MATERIAL{0};
        /** Descriptor binding index for the mesh surfaces buffer. */
        static constexpr vireo::DescriptorIndex BINDING_SURFACES{1};
        /** Descriptor binding index for the textures array/sampled images. */
        static constexpr vireo::DescriptorIndex BINDING_TEXTURE{2};
        /** Shared descriptor layout describing the above bindings. */
        inline static std::shared_ptr<vireo::DescriptorLayout> descriptorLayout{nullptr};

        /**
         * Constructs the resources container and allocates GPU-side structures.
         *
         * @param vireo        Graphics backend entry point.
         * @param config       High-level resources configuration (sizes, formats, etc.).
         * @param graphicQueue Queue used to submit upload and initialization work.
         */
        Resources(
            const vireo::Vireo& vireo,
            ResourcesConfiguration& config,
            const vireo::SubmitQueue& graphicQueue);

        /** Returns the device memory array storing vertex data. */
        DeviceMemoryArray& getVertexArray() { return vertexArray; }

        /** Returns the device memory array storing index data. */
        DeviceMemoryArray& getIndexArray() { return indexArray; }

        /** Returns the device memory array storing material parameters. */
        DeviceMemoryArray& getMaterialArray() { return materialArray; }

        /** Returns the device memory array storing mesh surface ranges/metadata. */
        DeviceMemoryArray& getMeshSurfaceArray() { return meshSurfaceArray; }

        /** Returns the global sampler collection used by the renderer. */
        Samplers& getSamplers() { return samplers; }

        /**
         * Adds a texture to the internal textures list and uploads it to the GPU.
         * If the capacity MAX_TEXTURES is reached, an engine-defined policy applies
         * (typically reuse or replace); callers should not rely on indices beyond the limit.
         *
         * @param image CPU-side image data to upload.
         * @return Texture index suitable for shader/resource binding.
         */
        uint32 addTexture(const Image& image);

        /** Returns the descriptor set that exposes resources to shaders. */
        const auto& getDescriptorSet() const { return descriptorSet; }

        /**
         * Flushes pending uploads to the device. Typically enqueues copy/transfer
         * commands to make new/updated resources visible to the GPU.
         */
        void flush();

        /** Applies incremental updates to buffers, images, and descriptors if needed. */
        void update();

        /** Releases all GPU and CPU-side resources owned by this container. */
        void cleanup();

        /** Marks the container as updated so dependent systems can react. */
        void setUpdated() { updated = true; }

        /** Returns true when an update has been flagged since the last processing. */
        bool isUpdated() const { return updated; }

        /** Provides access to the internal mutex used to guard mutations. */
        auto& getMutex() { return mutex; }

        /** Returns the default 2D blank image used as a safe fallback. */
        auto getBlankImage() { return blankImage; }

        /** Returns the default cubemap blank image used as a safe fallback. */
        auto getBlankCubeMap() { return blankCubeMap; }

        Resources(Resources&) = delete;
        Resources& operator = (Resources&) = delete;

    private:
        /** Reference to high-level configuration driving buffer sizes and formats. */
        const ResourcesConfiguration& config;
        /** Device memory array that stores all vertex buffers. */
        DeviceMemoryArray vertexArray;
        /** Device memory array that stores all index buffers. */
        DeviceMemoryArray indexArray;
        /** Device memory array that stores material parameter blocks. */
        DeviceMemoryArray materialArray;
        /** Device memory array that stores mesh surface descriptors. */
        DeviceMemoryArray meshSurfaceArray;
        /** Collection of pre-created sampler objects shared across materials. */
        Samplers samplers;
        /** Descriptor set bound at SET_RESOURCES with material/surfaces/textures. */
        std::shared_ptr<vireo::DescriptorSet> descriptorSet;
        /** List of GPU texture images managed by this container. */
        std::vector<std::shared_ptr<vireo::Image>> textures;
        /** Default 2D image used when a texture is missing. */
        std::shared_ptr<vireo::Image> blankImage;
        /** Default cubemap image used when a cubemap is missing. */
        std::shared_ptr<vireo::Image> blankCubeMap;
        /** Flag indicating the container has pending updates. */
        bool updated{false};
        /** Flag indicating that one or more textures changed and need syncing. */
        bool textureUpdated{false};
        /** Mutex to guard mutations to textures, buffers, and descriptor set. */
        std::mutex mutex;

        /** Creates a small in-memory JPEG used to initialize blank textures. */
        static std::vector<uint8> createBlankJPEG();

        /** Callback passed to stb to write encoded data into an external buffer. */
        static void stb_write_func(void *context, void *data, int size);
    };

}