// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// https://github.com/Nadrin/PBR/blob/master/data/shaders/glsl/pbr_fs.glsl
static const float PI = 3.1415926535897932384626433832795;
static const float Epsilon = 0.00001;
// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = float3(0.04);

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, const float alphaSq) {
    const float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k) {
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness) {
    const float r = roughness + 1.0;
    const float k = (r * r)  * 0.125; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta) {
    float cosTerm = 1.0 - cosTheta;
    float cosTermSq = cosTerm * cosTerm; // cosTerm^2
    float cosTerm5 = cosTermSq * cosTermSq * cosTerm; // cosTerm^5
    return F0 + (float3(1.0) - F0) * cosTerm5;
}
