/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.material;
#include <xxhash.h>

import lysa.application;

namespace lysa {

    Material::Material(const Type type, const std::wstring &name):
        Resource{name}, type{type} {
    }

    void Material::upload() {
        auto& resources = Application::getResources();
        if (!isUploaded()) {
            memoryBloc = resources.getMaterialArray().alloc(1);
        }
        const auto materialData = getMaterialData();
        resources.getMaterialArray().write(memoryBloc, &materialData);
    }

    MaterialData StandardMaterial::getMaterialData() const {
        auto data = MaterialData {
            .albedoColor = albedoColor,
            .pipelineId = getPipelineId(),
            .transparency = static_cast<int>(getTransparency()),
            .alphaScissor = getAlphaScissor(),
            .normalScale = normalScale,
        };
        if (diffuseTexture.texture) {
            data.diffuseTexture = {
                .index = static_cast<int32>(diffuseTexture.texture->getImage()->getIndex()),
                .samplerIndex = diffuseTexture.texture->getSamplerIndex(),
                .transform = float4x4{diffuseTexture.transform},
            };
        }
        if (normalTexture.texture) {
            data.normalTexture = {
                .index = static_cast<int32>(normalTexture.texture->getImage()->getIndex()),
                .samplerIndex = normalTexture.texture->getSamplerIndex(),
                .transform = float4x4{normalTexture.transform},
            };
        }
        return data;
    }

    StandardMaterial::StandardMaterial(const std::wstring &name):
        Material(STANDARD, name) {
    }

    void StandardMaterial::setAlbedoColor(const float4 &color) {
        albedoColor = color;
        setUpdated();
    }

    void StandardMaterial::setDiffuseTexture(const TextureInfo &texture) {
        diffuseTexture = texture;
        setUpdated();
    }

    void StandardMaterial::setNormalTexture(const TextureInfo &texture) {
        normalTexture = texture;
        setUpdated();
    }

    void StandardMaterial::setMetallicTexture(const TextureInfo &texture) {
        metallicTexture = texture;
        if (metallicFactor == -1.0f) { metallicFactor = 0.0f; }
        setUpdated();
    }

    void StandardMaterial::setRoughnessTexture(const TextureInfo &texture) {
        roughnessTexture = texture;
        if (metallicFactor == -1.0f) { metallicFactor = 0.0f; }
        setUpdated();
    }

    void StandardMaterial::setEmissiveTexture(const TextureInfo &texture) {
        emissiveTexture = texture;
        setUpdated();
    }

    void StandardMaterial::setMetallicFactor(const float metallic) {
        this->metallicFactor = metallic;
        setUpdated();
    }

    void StandardMaterial::setRoughnessFactor(const float roughness) {
        this->roughnessFactor = roughness;
        setUpdated();
    }

    void StandardMaterial::setEmissiveFactor(const float3& factor) {
        emissiveFactor = factor;
        setUpdated();
    }

    void StandardMaterial::setEmissiveStrength(const float strength) {
        emissiveStrength = strength;
        setUpdated();
    }

    void StandardMaterial::setNormalScale(const float scale) {
        normalScale = scale;
    }

    pipeline_id StandardMaterial::getPipelineId() const {
        const auto name = std::format("{0}{1}{2}",
            DEFAULT_PIPELINE_ID,
            static_cast<uint32>(getTransparency()),
            static_cast<uint32>(getCullMode()));
        return XXH32(name.c_str(), name.size(), 0);
    }

    ShaderMaterial::ShaderMaterial(const std::shared_ptr<ShaderMaterial> &orig):
        Material{SHADER, orig->getName()},
        fragFileName{orig->fragFileName},
        vertFileName{orig->vertFileName} {
        for (int i = 0; i < SHADER_MATERIAL_MAX_PARAMETERS; i++) {
            parameters[i] = orig->parameters[i];
        }
        upload();
        Application::getResources().setUpdated();
    }

    ShaderMaterial::ShaderMaterial(const std::wstring &fragShaderFileName,
                                   const std::wstring &vertShaderFileName,
                                   const std::wstring &name):
        Material{SHADER, name},
        fragFileName{fragShaderFileName},
        vertFileName{vertShaderFileName} {
    }

    void ShaderMaterial::setParameter(const int index, const float4& value) {
        parameters[index] = value;
        setUpdated();
    }

    uint32 ShaderMaterial::getPipelineId() const {
        const auto name = vertFileName + fragFileName;
        return XXH32(name.c_str(), name.size(), 0);
    }

    MaterialData ShaderMaterial::getMaterialData() const {
        return {
            .pipelineId = getPipelineId(),
            .parameters = {
                parameters[0],
                parameters[1],
                parameters[2],
                parameters[3],
            }
        };
    }
}
