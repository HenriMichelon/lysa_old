/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.material;

import lysa.application;

namespace lysa {

    Material::Material(const std::wstring &name):
        Resource(name) {
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
        return {
            .albedoColor = albedoColor,
        };
    }

    MaterialData ShaderMaterial::getMaterialData() const {
        return {
            .parameters = {
                parameters[0],
                parameters[1],
                parameters[2],
                parameters[3],
            }
        };
    }

    StandardMaterial::StandardMaterial(const std::wstring &name):
        Material(name) {
    }

    void StandardMaterial::setAlbedoColor(const float4 &color) {
        albedoColor = color;
        setUpdated();
    }

    void StandardMaterial::setAlbedoTexture(const TextureInfo &texture) {
        albedoTexture = texture;
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

    ShaderMaterial::ShaderMaterial(const std::shared_ptr<ShaderMaterial> &orig):
        Material{orig->getName()},
        fragFileName{orig->fragFileName},
        vertFileName{orig->vertFileName} {
        for (int i = 0; i < SHADER_MATERIAL_MAX_PARAMETERS; i++) {
            parameters[i] = orig->parameters[i];
        }
    }

    ShaderMaterial::ShaderMaterial(const std::wstring &fragShaderFileName,
                                   const std::wstring &vertShaderFileName,
                                   const std::wstring &name):
        Material{name},
        fragFileName{fragShaderFileName},
        vertFileName{vertShaderFileName} {
    }

    void ShaderMaterial::setParameter(const int index, const float4& value) {
        parameters[index] = value;
        setUpdated();
    }

}
