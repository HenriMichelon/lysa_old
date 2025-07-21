/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.assets_pack;

import vireo;
import lysa.math;
import lysa.types;
import lysa.nodes.node;
import lysa.resources.texture;

export namespace lysa {

    /*
     * Lysa assets pack binary file format containing resources for a scene : meshes, materials, textures and images.<br>
     * It can also be used as a complete scene file since it contains a node tree.<br>
     * This file format is adapted to Lysa and have the following advantages :<br>
     * - Binary file format : fast loading of data without deserialization
     * - Compressed images : images are compressed in GPU-compatible format like the BCn formats to reduce the VRAM usage.<br>
     * - One big images atlas : all images are read and directly uploaded to the GPU in one pass and without using a big CPU buffer.<br>
     * - Pre calculated mip levels : all images mips levels are pre-calculated and compressed.<br>
     * - Pre calculated data : all transforms are pre-calculated.<br>
     * - Pre calculated animations data : all animation data are made relative to the original object.<br>
     *<br>
     *%A Lysa assets pack file can be created from a glTF file using the `gltf2lysa` command line tool.<br>
     *<br>
     * **File format description :**
     * ```
     * Header : global header
     * array<ImageHeader + array<MipLevelInfo, mipLevels>, imagesCount> : images headers
     * array<TextureHeader, texturesCount> : textures headers
     * array<MaterialHeader, materialsCount> : materials headers
     * array<MeshHeader + array<SurfaceInfo, surfacesCount> + array<DataInfo, surfacesCount * uvsCount>, meshesCount> : meshes headers
     * array<NodeHeader + array<uint32, childrenCount>, nodesCount> : nodes headers
     * array<AnimationHeader + array<TrackInfo, tracksCount>, animationCount> : animation headers
     * uint32 : indicesCount
     * array<uint32, indicesCount> : indices data bloc
     * uint32 : positionsCount
     * array<float3, positionsCount> : positions data bloc
     * uint32 : normalsCount
     * array<float3, normalsCount> : normals data bloc
     * uint32 : uvsCount
     * array<vec2, uvsCount> : uvs data bloc
     * uint32 : tangentsCount
     * array<float4, tangentsCount> : tangents data bloc
     * array<float, keysCount> + array<float3, keyCount> for each track, for each animation : animations data
     * array<BCn compressed image, imagesCount> : images data bloc
     *  ```
     */
    class AssetsPack {
    public:
        /*
         * Maximum size of strings in files
         */
        static constexpr auto NAME_SIZE{64};

        /*
         * Magic header thing
         */
        static constexpr char MAGIC[]{ 'A', 'S', 'S', 'E', 'T', 'S' };

        /*
         * Current format version
         */
        static constexpr uint32 VERSION{1};

        /*
         * Global file header
         */
        struct Header {
            //! Magic header thing
            char   magic[6];
            //! Format version
            uint32 version{0};
            //! Total number of images
            uint32 imagesCount{0};
            //! Total number of textures
            uint32 texturesCount{0};
            //! Total number of materials
            uint32 materialsCount{0};
            //! Total number of meshes
            uint32 meshesCount{0};
            //! Total number of scene nodes
            uint32 nodesCount{0};
            uint32 animationsCount{0};
            //! Size in bytes of all the headers
            uint64 headersSize;
        };

        /*
         * Description of an image
         */
        struct ImageHeader {
            //! Image name, copied from the original file name
            char   name[NAME_SIZE];
            //! Image format (vireo::ImageFormat)
            uint32 format;
            //! Width in pixels
            uint32 width;
            //! Height in pixels
            uint32 height;
            //! Number of mips levels, and number of MipLevelInfo elements in the array following this struct
            uint32 mipLevels;
            //! Start of the image, relative to the start of the images data block
            uint64 dataOffset;
            //! Size in bytes of all levels
            uint64 dataSize;
        };

        /*
         * Description of a mip level for an image
         */
        struct MipLevelInfo {
            //! Start of the level, relative to the image dataOffset
            uint64 offset;
            //! Size in bytes of the level
            uint64 size;
        };

        /*
         * Description of a texture
         */
        struct TextureHeader {
            //! Associated image, -1 for a texture without image
            int32  imageIndex{-1};
            //! Minification filter to apply to texture lookups, [VkFilter format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFilter.html)
            uint32 minFilter;
            //! Magnification filter to apply to texture lookups, [VkFilter format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFilter.html)
            uint32 magFilter;
            //! Behavior of sampling with texture coordinates outside an image for U coordinates, [VkSamplerAddressMode format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
            uint32 samplerAddressModeU;
            //! Behavior of sampling with texture coordinates outside an image for V coordinates, [VkSamplerAddressMode format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
            uint32 samplerAddressModeV;
        };

        /*
         * Description of a texture attached to a material
         */
        struct TextureInfo {
            //! Attached texture, -1 if no texture
            int32    textureIndex{-1};
            //! Index of UV coordinates, in the UV DataInfo array of the attached surface
            uint32   uvsIndex;
            //! UV coordinates transform
            float3x3 transform;
        };

        /*
         * Description of a material
         */
        struct MaterialHeader {
            //! Material name
            char        name[NAME_SIZE];
            //! Culling mode CullMode format
            uint32      cullMode;
            //! Transparency, Transparency format
            uint32      transparency;
            //! Alpha scissor for transparency
            float       alphaScissor;
            //! Albedo color
            float4      albedoColor;
            //! Optional albedo texture
            TextureInfo albedoTexture;
            //! Metallic factor
            float       metallicFactor;
            //! Optional metallic texture
            TextureInfo metallicTexture;
            //! Roughness factor
            float       roughnessFactor;
            //! Optional roughness texture
            TextureInfo roughnessTexture;
            //! Emissive factor
            float3      emissiveFactor;
            //! Emissive streng
            float       emissiveStrength;
            //! Optional emissive texture
            TextureInfo emissiveTexture;
            //! Optional normal texture
            TextureInfo normalTexture;
            //! Normal scale
            float       normalScale;
        };

        /*
         * Description of a data bloc (indices, positions, normals, tangents, uv coords)
         */
        struct DataInfo {
            //! First index in the array
            uint32 first;
            //! Number of elements
            uint32 count;
        };

        /*
         * Description of a mesh primitive
         */
        struct SurfaceInfo {
            //! Attached material, -1 if no material
            int32    materialIndex{-1};
            //! Indices array
            DataInfo indices;
            //! Vertices positions array
            DataInfo positions;
            //! Normals array
            DataInfo normals;
            //! Tangents array
            DataInfo tangents;
            //! Number of DataInfo elements in the UV coordinates array of array following this struct
            uint32   uvsCount;
        };

        /*
         * Description of a mesh
         */
        struct MeshHeader {
            //! Name
            char   name[NAME_SIZE];
            //! Number of SurfaceInfo elements in the array following this struct
            uint32 surfacesCount;
        };

        /*
         * Description of a node in the scene
         */
        struct NodeHeader {
            //! Name
            char     name[NAME_SIZE];
            //! Associated mesh, -1 of no mesh. Nodes with mesh will be instantiated as z0::MeshInstance, other nodes as z0::Node
            uint32   meshIndex;
            //! World transform
            float4x4 transform;
            //! Number of children nodes, also the number of elements in the uint32 array following this struct
            uint32   childrenCount;
        };

        struct AnimationHeader {
            char   name[NAME_SIZE];
            uint32 tracksCount;
        };

        struct TrackInfo {
            int32  nodeIndex{-1};
            uint32 type;
            uint32 interpolation;
            uint32 keysCount;
            // + keyCount * float keyTime
            // + keyCount * variant<float3, quat> keyValue
        };

        /*
         * Load a scene from an assets pack file
         */
        static void load(Node& rootNode, const std::string &filename);

        /*
         * Load a scene from an assets pack data stream
         */
        static void load(Node& rootNode, std::ifstream &stream);

        AssetsPack() = default;

        static void print(const Header& header);
        static void print(const ImageHeader& header);
        static void print(const MipLevelInfo& header);
        static void print(const TextureHeader& header);
        static void print(const MaterialHeader& header);
        static void print(const MeshHeader& header);
        static void print(const SurfaceInfo& header);
        static void print(const DataInfo& header);

    protected:
        Header header{};
        std::vector<std::shared_ptr<Texture>> textures{};

        void loadScene(Node& rootNode, std::ifstream& stream);

        std::vector<std::shared_ptr<vireo::Image>> loadImagesAndTextures(
            const vireo::Buffer& stagingBuffer,
            const vireo::CommandList& commandList,
            std::ifstream& stream,
            const std::vector<ImageHeader>&,
            const std::vector<std::vector<MipLevelInfo>>&,
            const std::vector<TextureHeader>&);
    };

}
