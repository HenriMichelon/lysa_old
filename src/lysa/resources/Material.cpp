/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.material;

namespace lysa {

    Material::Material(const std::wstring &name):
        Resource(name) {
    }

    StandardMaterial::StandardMaterial(const std::wstring &name):
        Material(name) {
    }

    void StandardMaterial::setAlbedoTexture(const TextureInfo &texture) {
        albedoTexture = texture;
    }

    void StandardMaterial::setNormalTexture(const TextureInfo &texture) {
        normalTexture = texture;
    }

    void StandardMaterial::setMetallicTexture(const TextureInfo &texture) {
        metallicTexture = texture;
        if (metallicFactor == -1.0f) { metallicFactor = 0.0f; }
    }

    void StandardMaterial::setRoughnessTexture(const TextureInfo &texture) {
        roughnessTexture = texture;
        if (metallicFactor == -1.0f) { metallicFactor = 0.0f; }
    }

    void StandardMaterial::setEmissiveTexture(const TextureInfo &texture) {
        emissiveTexture = texture;
    }

    void StandardMaterial::setMetallicFactor(const float metallic) {
        this->metallicFactor = metallic;
    }

    void StandardMaterial::setRoughnessFactor(const float roughness) {
        this->roughnessFactor = roughness;
    }

    void StandardMaterial::setEmissiveFactor(const float3& factor) {
        emissiveFactor = factor;
    }

    void StandardMaterial::setEmissiveStrength(const float strength) {
        emissiveStrength = strength;
    }

    void StandardMaterial::setNormalScale(const float scale) {
        normalScale = scale;
    }

    ShaderMaterial::ShaderMaterial(const std::shared_ptr<ShaderMaterial> &orig):
        Material{orig->getName()},
        fragFileName{orig->fragFileName},
        vertFileName{orig->vertFileName} {
        for (int i = 0; i < MAX_PARAMETERS; i++) {
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
    }

}
