/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.assets_pack;

import vireo;
import lysa.application;
import lysa.virtual_fs;
import lysa.log;
import lysa.nodes.animation_player;
import lysa.nodes.mesh_instance;
import lysa.resources.animation;
import lysa.resources.animation_library;
import lysa.resources.image;
import lysa.resources.material;
import lysa.resources.mesh;
import lysa.resources.resource;

namespace lysa {

    void AssetsPack::load(Node& rootNode, const std::wstring &filename) {
        auto stream = VirtualFS::openReadStream(filename);
        return load(rootNode, stream);
    }

    void AssetsPack::load(Node& rootNode, std::ifstream &stream) {
        AssetsPack loader;
        loader.loadScene(rootNode, stream);
    }

    void AssetsPack::loadScene(Node& rootNode, std::ifstream& stream) {
        // Read the file global header
        stream.read(reinterpret_cast<std::istream::char_type *>(&header), sizeof(header));
        if (header.magic[0] != MAGIC[0] &&
            header.magic[1] != MAGIC[1] &&
            header.magic[2] != MAGIC[2] &&
            header.magic[3] != MAGIC[3] &&
            header.magic[4] != MAGIC[4] &&
            header.magic[5] != MAGIC[5]) {
            throw Exception("Assets pack bad magic");
        }
        if (header.version != VERSION) {
            throw Exception("Assets pack version");
        }
        // print(header);

        // Read the images & mips levels headers
        auto imageHeaders = std::vector<ImageHeader>(header.imagesCount);
        auto levelHeaders = std::vector<std::vector<MipLevelInfo>>(header.imagesCount);
        uint64_t totalImageSize{0};
        for (auto imageIndex = 0; imageIndex < header.imagesCount; ++imageIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&imageHeaders[imageIndex]), sizeof(ImageHeader));
            // print(imageHeaders[imageIndex]);
            levelHeaders[imageIndex].resize(imageHeaders[imageIndex].mipLevels);
            stream.read(reinterpret_cast<std::istream::char_type *>(levelHeaders[imageIndex].data()), sizeof(MipLevelInfo) * imageHeaders[imageIndex].mipLevels);
            totalImageSize += imageHeaders[imageIndex].dataSize;
        }

        // Read the textures & materials headers
        auto textureHeaders = std::vector<TextureHeader>(header.texturesCount);
        if (header.texturesCount > 0) {
            stream.read(reinterpret_cast<std::istream::char_type *>(textureHeaders.data()), textureHeaders.size() * sizeof(TextureHeader));
        }
        auto materialHeaders = std::vector<MaterialHeader>(header.materialsCount);
        if (header.materialsCount > 0) {
            stream.read(reinterpret_cast<std::istream::char_type *>(materialHeaders.data()), materialHeaders.size() * sizeof(MaterialHeader));
        }

        // Read the meshes & surfaces headers
        auto meshesHeaders = std::vector<MeshHeader>(header.meshesCount);
        auto surfaceInfo = std::vector<std::vector<SurfaceInfo>> {header.meshesCount};
        auto uvsInfos = std::vector<std::vector<std::vector<DataInfo>>> {header.meshesCount};
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&meshesHeaders[meshIndex]), sizeof(MeshHeader));
            // print(meshesHeaders[meshIndex]);
            surfaceInfo[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            uvsInfos[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            for (auto surfaceIndex = 0; surfaceIndex < meshesHeaders[meshIndex].surfacesCount; ++surfaceIndex) {
                stream.read(reinterpret_cast<std::istream::char_type *>(&surfaceInfo[meshIndex][surfaceIndex]), sizeof(SurfaceInfo));
                // print(surfaceInfo[meshIndex][surfaceIndex]);
                uvsInfos[meshIndex][surfaceIndex].resize(surfaceInfo[meshIndex][surfaceIndex].uvsCount);
                stream.read(reinterpret_cast<std::istream::char_type *>(uvsInfos[meshIndex][surfaceIndex].data()), sizeof(DataInfo) * uvsInfos[meshIndex][surfaceIndex].size());
            }
        }

        // Read the nodes headers
        auto nodeHeaders = std::vector<NodeHeader>(header.nodesCount);
        auto childrenIndexes = std::vector<std::vector<uint32>>(header.nodesCount);
        for (auto nodeIndex = 0; nodeIndex < header.nodesCount; ++nodeIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&nodeHeaders.at(nodeIndex)), sizeof(NodeHeader));
            childrenIndexes.at(nodeIndex).resize(nodeHeaders.at(nodeIndex).childrenCount);
            stream.read(reinterpret_cast<std::istream::char_type *>(childrenIndexes.at(nodeIndex).data()), sizeof(uint32) * childrenIndexes[nodeIndex].size());
        }

        auto animationHeaders = std::vector<AnimationHeader>(header.animationsCount);
        auto tracksInfos = std::vector<std::vector<TrackInfo>> (header.animationsCount);
        for (auto animationIndex = 0; animationIndex < header.animationsCount; ++animationIndex) {
            stream.read(reinterpret_cast<std::istream::char_type *>(&animationHeaders.at(animationIndex)), sizeof(AnimationHeader));
            tracksInfos[animationIndex].resize(animationHeaders[animationIndex].tracksCount);
            stream.read(reinterpret_cast<std::istream::char_type *>(tracksInfos[animationIndex].data()), sizeof(TrackInfo) * tracksInfos[animationIndex].size());
        }

        // Read the meshes data
        uint32 count{0};
        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<uint32> indices(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(indices.data()), count * sizeof(uint32));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float3> positions(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(positions.data()), count * sizeof(float3));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float3> normals(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(normals.data()), count * sizeof(float3));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float2> uvs(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(uvs.data()), count * sizeof(float2));

        stream.read(reinterpret_cast<std::istream::char_type *>(&count), sizeof(uint32));
        std::vector<float4> tangents(count);
        stream.read(reinterpret_cast<std::istream::char_type *>(tangents.data()), count * sizeof(float4));

        // INFO(std::format("{} indices, {} positions, {} normals, {} uvs, {} tangents",
            // indices.size(), positions.size(), normals.size(), uvs.size(), tangents.size()));
        // for(auto& pos : positions) {
        //     log(to_string(pos));
        // }

        // Read the animations data
        auto animationPlayers = std::map<uint32, std::shared_ptr<AnimationPlayer>>{};
        for (auto animationIndex = 0; animationIndex < header.animationsCount; animationIndex++) {
            auto anim = std::make_shared<Animation>(animationHeaders[animationIndex].tracksCount, to_wstring(animationHeaders[animationIndex].name));
            for (auto trackIndex = 0; trackIndex < animationHeaders[animationIndex].tracksCount; trackIndex++) {
                auto animationPlayer = std::shared_ptr<AnimationPlayer>{};
                auto& trackInfo = tracksInfos[animationIndex][trackIndex];
                auto nodeIndex = trackInfo.nodeIndex;
                if (animationPlayers.contains(nodeIndex)) {
                    animationPlayer = animationPlayers[nodeIndex];
                } else {
                    animationPlayer = std::make_shared<AnimationPlayer>(); // node association is made later
                    animationPlayer->add(L"", std::make_shared<AnimationLibrary>());
                    animationPlayers[nodeIndex] = animationPlayer;
                }
                animationPlayer->getLibrary()->add(anim->getName(), anim);
                animationPlayer->setCurrentAnimation(anim->getName());
                auto& track = anim->getTrack(trackIndex);
                track.type = static_cast<AnimationType>(trackInfo.type);
                track.interpolation = static_cast<AnimationInterpolation>(trackInfo.interpolation);
                track.keyTime.resize(trackInfo.keysCount);
                stream.read(reinterpret_cast<std::istream::char_type *>(track.keyTime.data()), trackInfo.keysCount * sizeof(float));
                track.duration = track.keyTime.back() + track.keyTime.front();
                track.keyValue.resize(trackInfo.keysCount);
                stream.read(reinterpret_cast<std::istream::char_type *>(track.keyValue.data()), trackInfo.keysCount * sizeof(float3));
            }
        }

        // Read, upload and create the Image and Texture objets (Vireo specific)
        if (header.imagesCount > 0) {
            loadImagesAndTextures(stream, imageHeaders, levelHeaders, textureHeaders, totalImageSize);
        }

        // Create the Material objects
        std::vector<std::shared_ptr<Material>> materials{header.materialsCount};
        std::map<unique_id, int> materialsTexCoords;
        for (auto materialIndex = 0; materialIndex < header.materialsCount; ++materialIndex) {
            auto& header = materialHeaders.at(materialIndex);
            auto material = std::make_shared<StandardMaterial>(to_wstring(header.name));
            auto textureInfo = [&](const TextureInfo& info) {
                auto texInfo = StandardMaterial::TextureInfo {
                    .texture = info.textureIndex == -1 ? nullptr : dynamic_pointer_cast<ImageTexture>(textures.at(info.textureIndex)),
                    .transform = info.transform,
                };
                materialsTexCoords[material->getId()] = info.uvsIndex;
                return texInfo;
            };
            material->setCullMode(static_cast<vireo::CullMode>(header.cullMode));
            material->setTransparency(static_cast<Transparency>(header.transparency));
            material->setAlphaScissor(header.alphaScissor);
            material->setAlbedoColor(header.albedoColor);
            material->setAlbedoTexture(textureInfo(header.albedoTexture));
            material->setMetallicFactor(header.metallicFactor);
            material->setMetallicTexture(textureInfo(header.metallicTexture));
            material->setRoughnessFactor(header.roughnessFactor);
            material->setRoughnessTexture(textureInfo(header.roughnessTexture));
            material->setEmissiveFactor(header.emissiveFactor);
            material->setEmissiveStrength(header.emissiveStrength);
            material->setEmissiveTexture(textureInfo(header.emissiveTexture));
            material->setNormalTexture(textureInfo(header.normalTexture));
            material->setNormalScale(header.normalScale);
            materials.at(materialIndex) = material;
        }

        // Create the Mesh, Surface & Vertex objects
        std::vector<std::shared_ptr<Mesh>> meshes{header.meshesCount};
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            auto& header   = meshesHeaders.at(meshIndex);
            auto  mesh     = std::make_shared<Mesh>(to_wstring(header.name));
            auto &meshVertices = mesh->getVertices();
            auto &meshIndices  = mesh->getIndices();
            // print(header);
            for (auto surfaceIndex = 0; surfaceIndex < header.surfacesCount; ++surfaceIndex) {
                auto &info = surfaceInfo.at(meshIndex)[surfaceIndex];
                // print(info);
                auto firstIndex = meshIndices.size();
                auto firstVertex  = meshVertices.size();
                auto surface = std::make_shared<MeshSurface>(firstIndex, info.indices.count);
                // Load indices
                meshIndices.reserve(meshIndices.size() + info.indices.count);
                for(auto i = 0; i < info.indices.count; ++i) {
                    meshIndices.push_back(indices[info.indices.first + i]);
                }
                // Load positions
                meshVertices.resize(meshVertices.size() + info.positions.count);
                for(auto i = 0; i < info.positions.count; ++i) {
                    meshVertices[firstVertex + i] = {
                        .position = positions[info.positions.first + i],
                    };
                }
                // Load normals
                for(auto i = 0; i < info.normals.count; ++i) {
                    meshVertices[firstVertex + i].normal = normals[info.normals.first + i];
                }
                // Load tangents
                for(auto i = 0; i < info.tangents.count; ++i) {
                    meshVertices[firstVertex + i].tangent = tangents[info.tangents.first + i];
                }
                if (info.materialIndex != -1) {
                    // associate material to surface & mesh
                    const auto& material = materials[info.materialIndex];
                    surface->material = material;
                    mesh->getMaterials().insert(material);
                    // load UVs
                    auto texCoord = 0;
                    if (materialsTexCoords.contains(material->getId())) {
                        texCoord = materialsTexCoords.at(material->getId());
                    }
                    const auto& texCoordInfo = uvsInfos.at(meshIndex)[surfaceIndex][texCoord];
                    for(auto i = 0; i < texCoordInfo.count; i++) {
                        meshVertices[firstVertex + i].uv = uvs[texCoordInfo.first + i];
                    }
                } else {
                    // Mesh have no material, use a default one
                    const auto &material = std::make_shared<StandardMaterial>();
                    surface->material = material;
                    mesh->getMaterials().insert(material);
                }
                // calculate missing tangents
                if (info.tangents.count == 0) {
                    for (auto i = 0; i < meshIndices.size(); i += 3) {
                        auto &vertex1  = meshVertices[meshIndices[i]];
                        auto &vertex2  = meshVertices[meshIndices[i + 1]];
                        auto &vertex3  = meshVertices[meshIndices[i + 2]];
                        float3 edge1    = vertex2.position - vertex1.position;
                        float3 edge2    = vertex3.position - vertex1.position;
                        float2 deltaUV1 = vertex2.uv - vertex1.uv;
                        float2 deltaUV2 = vertex3.uv - vertex1.uv;

                        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                        float3  tangent{
                            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
                        };
                        vertex1.tangent = float4{tangent, 1.0};
                        vertex2.tangent = float4{tangent, 1.0};
                        vertex3.tangent = float4{tangent, 1.0};
                    }
                }
                mesh->getSurfaces().push_back(surface);
            }
            mesh->upload();
            meshes[meshIndex] = mesh;
        }

        // Create the Node objects
        std::vector<std::shared_ptr<Node>> nodes{(header.nodesCount)};
        for (auto nodeIndex = 0; nodeIndex < header.nodesCount; ++nodeIndex) {
            std::shared_ptr<Node> newNode;
            std::wstring          name{to_wstring(nodeHeaders[nodeIndex].name)};
            // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the
            // MeshInstance class
            if (nodeHeaders[nodeIndex].meshIndex != -1) {
                auto mesh = meshes[nodeHeaders.at(nodeIndex).meshIndex];
                newNode = std::make_shared<MeshInstance>(mesh, name);
            } else {
                newNode = std::make_shared<Node>(name);
            }
            newNode->setTransformLocal(nodeHeaders.at(nodeIndex).transform);
            nodes[nodeIndex] = newNode;
        }

        for (auto animationIndex = 0; animationIndex < header.animationsCount; animationIndex++) {
            for (auto trackIndex = 0; trackIndex < animationHeaders[animationIndex].tracksCount; trackIndex++) {
                auto nodeIndex = tracksInfos[animationIndex][trackIndex].nodeIndex;
                auto& player = animationPlayers[nodeIndex];
                if (!player->getParent()) {
                    auto& node = nodes[nodeIndex];
                    // player->setNode(node);
                    node->addChild(player);
                }
            }
        }

        // Build the scene tree
        for (auto nodeIndex = 0; nodeIndex < header.nodesCount; ++nodeIndex) {
            auto& sceneNode = nodes[nodeIndex];
            for (auto i = 0; i < nodeHeaders.at(nodeIndex).childrenCount; i++) {
                sceneNode->addChild(nodes[childrenIndexes.at(nodeIndex)[i]]);
            }
        }

        // find the top nodes, with no parents
        for (auto &node : nodes) {
            if (node->getParent() == nullptr) {
                rootNode.addChild(node);
            }
        }
    }

    void AssetsPack::loadImagesAndTextures(
        std::ifstream &stream,
        const std::vector<ImageHeader>& imageHeaders,
        const std::vector<std::vector<MipLevelInfo>>&levelHeaders,
        const std::vector<TextureHeader>& textureHeaders,
        const uint64 totalImageSize) {
        auto& vireo = Application::getVireo();

        // Upload all images into VRAM using one big staging buffer
        const auto& textureStagingBuffer = vireo.createBuffer(
            vireo::BufferType::IMAGE_UPLOAD,
            totalImageSize,
            1,
            L"textureStagingBuffer");
        textureStagingBuffer->map();
        const auto commandAllocator = vireo.createCommandAllocator(vireo::CommandType::GRAPHIC);
        const auto commandList = commandAllocator->createCommandList();
        commandList->begin();
        static constexpr size_t BLOCK_SIZE = 64 * 1024;
        auto transferBuffer = std::vector<char> (BLOCK_SIZE);
        auto transferOffset = size_t{0};
        while (stream.read(transferBuffer.data(), BLOCK_SIZE) || stream.gcount() > 0) {
            const auto bytesRead = stream.gcount();
            textureStagingBuffer->write(transferBuffer.data(), bytesRead, transferOffset);
            transferOffset += bytesRead;
        }
        std::printf("%llu bytes read\n", transferOffset);

        // Create all images from this staging buffer
        std::vector<std::shared_ptr<vireo::Image>> images;
        for (auto textureIndex = 0; textureIndex < header.texturesCount; ++textureIndex) {
            const auto& texture = textureHeaders[textureIndex];
            if (texture.imageIndex != -1) {
                const auto& imageHeader = imageHeaders[texture.imageIndex];
                INFO("Loading image ", imageHeader.name, " ", imageHeader.width, "x", imageHeader.height, " ", imageHeader.format);
                print(imageHeader);
                const auto name = to_wstring(imageHeader.name);
                const auto image = vireo.createImage(
                    static_cast<vireo::ImageFormat>(imageHeader.format),
                    imageHeader.width,
                    imageHeader.height,
                    imageHeader.mipLevels,
                    1,
                    name);
                commandList->barrier(
                    image,
                    vireo::ResourceState::UNDEFINED,
                    vireo::ResourceState::COPY_DST,
                    0,
                    imageHeader.mipLevels);
                auto sourceOffsets = std::vector<size_t>(imageHeader.mipLevels);
                for (int mipLevel = 0; mipLevel < imageHeader.mipLevels; ++mipLevel) {
                    sourceOffsets[mipLevel] = levelHeaders[texture.imageIndex][mipLevel].offset;
                }
                commandList->copy(
                    textureStagingBuffer,
                    image,
                    sourceOffsets);
                // // We must copy mip level per mip level because of memory alignment for compressed formats
                // for (uint32 mip_level = 0; mip_level < imageHeader.mipLevels; mip_level++) {
                //     commandList->copy(
                //         textureStagingBuffer,
                //         image,
                //         imageHeader.dataOffset + levelHeaders[texture.imageIndex][mip_level].offset,
                //         mip_level);
                // }
                commandList->barrier(
                    image,
                    vireo::ResourceState::COPY_DST,
                    vireo::ResourceState::UNDEFINED,
                    0,
                    imageHeader.mipLevels);
                textures.push_back(std::make_shared<ImageTexture>(std::make_shared<Image>(image, name)));
            }
        }
        commandList->end();
        Application::getGraphicQueue()->submit({commandList});
        Application::getGraphicQueue()->waitIdle();
    }

    void AssetsPack::print(const Header& header) {
        std::printf("Version : %d\nImages count : %d\nTextures count : %d\nMaterials count : %d\nMeshes count : %d\nNodes count : %d\nAnimations count : %d\nHeaders size : %llu\n",
            header.version,
            header.imagesCount,
            header.texturesCount,
            header.materialsCount,
            header.meshesCount,
            header.nodesCount,
            header.animationsCount,
            header.headersSize);
    }
    void AssetsPack::print(const ImageHeader& header) {
        std::printf("Name : %s\nFormat : %d\nWidth : %d\nHeight : %d\nLevels : %d\ndataOffset : %llu\ndataSize : %llu\n",
            header.name,
            header.format,
            header.width,
            header.height,
            header.mipLevels,
            header.dataOffset,
            header.dataSize);
    }
    void AssetsPack::print(const MipLevelInfo& header) {
        std::printf("Offset : %llu\nSize: %llu\n", header.offset, header.size);
    }

    void AssetsPack::print(const TextureHeader& header) {
        std::printf("imageIndex : %d\nminFilter : %d\nmagFilter : %d\nsampleAddressModeU : %d\nsampleAddressModeV : %d\n",
            header.imageIndex,
            header.minFilter,
            header.magFilter,
            header.samplerAddressModeU,
            header.samplerAddressModeV);
    }

    void AssetsPack::print(const MaterialHeader& header) {
        std::printf("Name : %s\n", header.name);
    }

    void AssetsPack::print(const MeshHeader& header) {
        std::printf("Name : %s\nSurfaces count : %d\n", header.name, header.surfacesCount);
    }

    void AssetsPack::print(const DataInfo& header) {
        std::printf("First : %d\nCount : %d\n", header.first, header.count);
    }

    void AssetsPack::print(const SurfaceInfo& header) {
        std::printf("materialIndex : %d\nindices : %d,%d\npositions : %d,%d\nnormals : %d,%d\ntangents : %d,%d\nuvsCount : %d\n",
            header.materialIndex,
            header.indices.first, header.indices.count,
            header.positions.first, header.positions.count,
            header.normals.first, header.normals.count,
            header.tangents.first, header.tangents.count,
            header.uvsCount);
    }

}
