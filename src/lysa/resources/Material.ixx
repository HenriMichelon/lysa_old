/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.material;

import vireo;
import lysa.constants;
import lysa.enums;
import lysa.math;
import lysa.memory;
import lysa.types;
import lysa.resources.resource;
import lysa.resources.texture;

export namespace lysa {

    struct TextureInfoData {
        int32    index{-1};
        uint32   samplerIndex{0};
        float4x4 transform{1.0f};
    };

    struct MaterialData {
        float4 albedoColor{0.9f, 0.0f, 0.6f, 1.0f};
        uint32 pipelineId;

        int    transparency{0};
        float  alphaScissor{0.1f};
        float  normalScale{1.0f};

        float  metallicFactor{0.0f};
        float  roughnessFactor{1.0f};
        float4 emissiveFactor{0.0f, 0.0f, 0.0f, 1.0f}; // factor + strength

        TextureInfoData diffuseTexture{};
        TextureInfoData normalTexture{};
        TextureInfoData metallicTexture{};
        TextureInfoData roughnessTexture{};
        TextureInfoData emissiveTexture{};
        float4 parameters[SHADER_MATERIAL_MAX_PARAMETERS]{};
    };

    /**
     * Base class for all materials of models surfaces
     */
    class Material : public Resource {
    public:
        enum Type {
            STANDARD,
            SHADER
        };
        /**
         * Returns the cull mode.
         */
        auto getCullMode() const { return cullMode; }

        /**
         * Sets the CullMode.
         * Determines which side of the triangle to cull depending on whether the triangle faces towards or away from the camera.
         */
        void setCullMode(const vireo::CullMode mode) { cullMode = mode; }

        /**
         * Returns the transparency mode
         */
        auto getTransparency() const { return transparency; }

        /**
         * Sets the transparency mode
         */
        void setTransparency(const Transparency transparencyMode) { transparency = transparencyMode; }

        /**
         * Returns the alpha scissor threshold value
         */
        auto getAlphaScissor() const { return alphaScissor; }

        /**
         * Sets the alpha scissor threshold value
         * Threshold at which the alpha scissors will discard values.
         * Higher values will result in more pixels being discarded.
         * If the material becomes too opaque at a distance, try increasing this value.
         * If the material disappears at a distance, try decreasing this value.
         */
        void setAlphaScissor(const float scissor) { alphaScissor = scissor; }

        auto isUploaded() const { return memoryBloc.size > 0; }

        void upload();

        virtual MaterialData getMaterialData() const = 0;

        virtual pipeline_id getPipelineId() const = 0;

        const auto& getMaterialIndex() const { return memoryBloc.instanceIndex; }

        auto getType() const { return type; }

        void setBypassUpload(const bool bypass) { bypassUpload = bypass; }

    protected:
        Material(Type type, const std::string &name);

    private:
        const Type      type;
        vireo::CullMode cullMode{vireo::CullMode::NONE};
        Transparency    transparency{Transparency::DISABLED};
        float           alphaScissor{0.1f};
        MemoryBlock     memoryBloc;
        bool            bypassUpload{false};
    };

    /**
     * Simple albedo/specular/normal material
     */
    class StandardMaterial : public Material {
    public:
        /**
         * References and properties of a texture
         */
        struct TextureInfo {
            std::shared_ptr<ImageTexture> texture{nullptr};
            float3x3                      transform{float3x3::identity()};
        };

        /**
         * Creates a StandardMaterial with default parameters
         */
        StandardMaterial(const std::string &name = "StandardMaterial");

        /**
         * Returns the material's base color.
         */
        const auto& getAlbedoColor() const { return albedoColor; }

        /**
         * Sets the material's base color.
         */
        void setAlbedoColor(const float4 &color);

        /**
         * Returns the diffuse texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        const auto& getDiffuseTexture() const { return diffuseTexture; }

        /**
         * Sets the diffuse texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        void setDiffuseTexture(const TextureInfo &texture);

        /**
         * Returns the normal texture
         */
        const auto &getNormalTexture() const { return normalTexture; }

        /**
         * Sets the normal texture
         */
        void setNormalTexture(const TextureInfo &texture);

        /**
         * Returns the metallic factor
         */
        auto getMetallicFactor() const { return metallicFactor; }

        /**
         * Sets the metallic factor
         */
        void setMetallicFactor(float metallic);

        /**
         * Return the metallic image texture. Only the BLUE channel is used by the default shader.
         */
        const auto& getMetallicTexture() const { return metallicTexture; }

        /**
         * Sets the metallic image texture. Only the BLUE channel is used by the default shader.
         */
        void setMetallicTexture(const TextureInfo &texture);

        /**
         * Returns the roughness factor
         */
        float getRoughnessFactor() const { return roughnessFactor; }

        /**
         * Sets the roughness factor
         */
        void setRoughnessFactor(float roughness);

        /**
         * Returns the roughness image texture. Only the RED channel is used by the default shader.
         */
        const auto& getRoughnessTexture() const { return roughnessTexture; }

        /**
         * Sets the roughness image texture. Only the RED channel is used by the default shader.
         */
        void setRoughnessTexture(const TextureInfo &texture);

        /**
         * Returns the emissive colors image texture
         */
        const auto& getEmissiveTexture() const { return emissiveTexture; }

        /**
         * Return the emissive colors factor
         */
        float3 getEmissiveFactor() const { return emissiveFactor; }

        /**
         * Sets the emissive colors factor
         */
        void setEmissiveFactor(const float3& emissive);

        /**
         * Return the emissive colors strength
         */
        float getEmissiveStrength() const { return emissiveStrength; }

        /**
         * Sets the emissive colors strength
         */
        void setEmissiveStrength(float strength);

        /**
         * Sets the emissive colors image texture. Used as a linear RGB texture by the default shader.
         */
        void setEmissiveTexture(const TextureInfo& texture);

        /**
         * Returns the scale applied to the normal image texture.
         * See https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_normaltextureinfo_scale
         */
        auto getNormalScale() const { return normalScale; }

        /**
         * Sets the scale applied to the normal image texture.
         * See https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_normaltextureinfo_scale
         */
        void setNormalScale(float scale);

        MaterialData getMaterialData() const override;

        pipeline_id getPipelineId() const override;

    private:
        float4       albedoColor{1.0f, 0.0f, 0.5f, 1.0f};
        TextureInfo  diffuseTexture{};
        float        metallicFactor{0.0f};
        TextureInfo  metallicTexture{};
        float        roughnessFactor{1.0f};
        TextureInfo  roughnessTexture{};
        float3       emissiveFactor{0.0f};
        float        emissiveStrength{1.0f};
        TextureInfo  emissiveTexture;
        TextureInfo  normalTexture{};
        float        normalScale{1.0f};
    };

    /**
     * Shader-based material
     */
    class ShaderMaterial : public Material {
    public:
        /**
         * Creates a ShaderMaterial by copy
         */
        ShaderMaterial(const std::shared_ptr<ShaderMaterial> &orig);

        /**
         * Creates a ShaderMaterial
         * @param fragShaderFileName fragment shader file path, relative to the application directory
         * @param vertShaderFileName vertex shader file path, relative to the application directory
         * @param name Resource name
         */
        ShaderMaterial(const std::string &fragShaderFileName,
                       const std::string &vertShaderFileName = "",
                       const std::string &name               = "ShaderMaterial");

        pipeline_id getPipelineId() const override;

        /**
         * Returns the fragment shader file path, relative to the application directory
         */
        const auto& getFragFileName() const { return fragFileName; }

        /**
         * Returns the vertex shader file path, relative to the application directory
         */
        const auto& getVertFileName() const { return vertFileName; }

        /**
         * Sets a parameter value
         */
        void setParameter(int index, const float4& value);

        /**
         * Returns a parameter value
         */
        auto getParameter(const int index) const { return parameters[index]; }

        MaterialData getMaterialData() const override;

    private:
        const std::string fragFileName;
        const std::string vertFileName;
        float4             parameters[SHADER_MATERIAL_MAX_PARAMETERS]{};
    };

}
