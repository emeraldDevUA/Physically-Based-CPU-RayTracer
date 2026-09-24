#include <glm/glm.hpp>

#include "shading/ggx_material.h"

#include <ostream>

#include "core/constants.h"
#include "core/random.h"

#include "shading/auxhiliary_methods.h"

GGXMaterial::GGXMaterial(const std::shared_ptr<Texture>& albedo)
{
    this->m_params.albedo = albedo;
}

GGXMaterial::GGXMaterial(Params params) : m_params(std::move(params))
{
}

Color GGXMaterial::albedo(const HitRecord& rec) const
{
    if (!m_params.albedo)
        return Color(0.0);

    return this->m_params.albedo->value(rec.uv, rec.position);
}

double GGXMaterial::roughness(const HitRecord& rec) const
{
    if (!m_params.roughness)
        return 0.5;

    return std::max(
        this->m_params.roughness->value(rec.uv, rec.position).x, 0.03);
}

double GGXMaterial::metallic(const HitRecord& rec) const
{
    return this->m_params.metallic->value(rec.uv, rec.position).x;
}

Color GGXMaterial::emission(const HitRecord& rec) const
{
    if (!m_params.emission)
        return Color(0.0);

    return this->m_params.emission->value(rec.uv, rec.position);
}

Color GGXMaterial::ambient_occlusion(const HitRecord& rec) const
{
    if (!m_params.ambient_occlusion)
        return Color(1.0);

    return this->m_params.ambient_occlusion->value(rec.uv, rec.position);
}


dvec3 GGXMaterial::normal(const HitRecord& rec) const
{
    if (!m_params.normal)
        return rec.geometricNormal; // no normal map — use geometric normal

    // Sample tangent-space normal and remap [0,1] → [-1,1]
    const Color raw = m_params.normal->value(rec.uv, rec.position);
    const dvec3 n = dvec3(raw.r, raw.g, raw.b) * 2.0 - 1.0;

    // Transform from tangent space to world space via TBN
    const dvec3 N = normalize(n);
    dvec3 T, B;
    buildTBN(N, T, B);

    return normalize(T * n.x + B * n.y + N * n.z);
}

MaterialSample GGXMaterial::sample(const HitRecord& rec, const dvec3& wo) const
{
    const dvec3 N = normal(rec);
    const Color alb = albedo(rec);
    const Color emi = emission(rec);

    // ── Transmission path — dark albedo acts as glass ─────────────────────
    if (double lum = toGrayscale(alb); lum < 0.09)
    {
        constexpr double ior = 2.42;

        bool frontFace = glm::dot(wo, N) > 0.0;
        double etaRatio = frontFace ? (1.0 / ior) : ior;
        dvec3 n = frontFace ? N : -N;

        dvec3 unitWo = normalize(wo);
        double cosTheta = clamp(glm::dot(unitWo, n), 0.0, 1.0);
        double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

        bool tir = etaRatio * sinTheta > 1.0;
        double reflectance = schlick(cosTheta, etaRatio);

        dvec3 wi;
        if (tir || reflectance > randomDouble())
        {
            wi = glm::reflect(-unitWo, n);
        }
        else
        {
            wi = glm::refract(-unitWo, n, etaRatio);
        }

        Color transmitColor = lum < 0.01 ? Color(1.0)
                                  : glm::mix(emi, alb, lum / 0.1);

        MaterialSample ms;
        ms.wi = wi;
        ms.weight = transmitColor;
        ms.pdf = 1.0;
        ms.valid = true;
        ms.delta = true;

        return ms;
    }

    // ── Normal GGX reflection path ─────────────────────────────────────────
    const double r1 = randomDouble();
    const double r2 = randomDouble();

    dvec3 T, B;
    buildTBN(N, T, B);

    double m_roughness = roughness(rec);
    const double a = m_roughness * m_roughness;
    const double theta = std::atan(a * std::sqrt(r1) / std::sqrt(1.0 - r1));
    const double phi = 2.0 * constants::kPi * r2;


    const double sinTheta = std::sin(theta);
    const double cosTheta = std::cos(theta);

    const dvec3 localH = {sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta};

    const dvec3 H = normalize(T * localH.x + B * localH.y + N * localH.z);
    const dvec3 wi = normalize(2.0 * glm::dot(wo, H) * H - wo);

    MaterialSample ms;
    ms.wi = wi;
    ms.delta = false;

    const double NdotL = std::max(glm::dot(N, wi), 0.0);
    const double NdotV = std::max(glm::dot(N, wo), 0.0);

    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        ms.weight = Color(0.0);

        return ms;
    }

    const double p = pdf(rec, wo, wi);
    if (p <= 0.0)
    {
        ms.weight = Color(0.0);
        return ms;
    }

    ms.pdf = p;  // add this
    ms.weight = evaluate(rec, wo, wi) / p;
    ms.valid = true;

    // Guard against weight spikes from near-zero pdf
    if (glm::any(glm::isnan(ms.weight)) || glm::any(glm::isinf(ms.weight)))
    {
        ms.valid = false;
    }

    return ms;
}

double GGXMaterial::pdf(const HitRecord& rec,
                        const dvec3& wo,
                        const dvec3& wi) const
{
    const dvec3 N = normalize(rec.geometricNormal);
    const dvec3 H = normalize(wo + wi); // half vector

    const double NdotH = std::max(glm::dot(N, H), 0.0);
    const double VdotH = std::max(glm::dot(wo, H), 0.0);

    const double m_roughness = roughness(rec);
    const double a = m_roughness * m_roughness;
    const double a2 = a * a;

    // GGX NDF
    const double denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    const double D = a2 / (constants::kPi * denom * denom);

    // Convert from half-vector pdf to wi pdf
    // p(wi) = D(H) * NdotH / (4 * VdotH)
    return (D * NdotH) / (4.0 * VdotH + 1e-6);
}

Color GGXMaterial::evaluate(const HitRecord& rec,
                            const dvec3& wo,
                            const dvec3& wi) const
{
    const dvec3 N = normal(rec);
    const dvec3 H = normalize(wo + wi); // half-vector

    const double NoV = std::max(glm::dot(N, wo), 0.0);
    const double NoL = std::max(glm::dot(N, wi), 0.0);
    const double NoH = std::max(glm::dot(N, H), 0.0);
    const double VoH = std::max(glm::dot(wo, H), 0.0);

    if (NoV <= 0.0 || NoL <= 0.0)
        return Color(0.0);

    const double a = roughness(rec);
    const double a2 = a * a;
    const double m = metallic(rec);
    const Color baseColor = albedo(rec);

    // --- D: GGX Normal Distribution ---
    const double denom = (NoH * NoH * (a2 - 1.0) + 1.0);
    if (denom < constants::kEpsilon)
        return Color(0.0); // prevent D blowing up

    const double D = a2 / (constants::kPi * denom * denom);

    // --- F: Fresnel-Schlick ---
    // Dialectrics use F0 = 0.04; metals use the albedo as F0
    const Color F0 = glm::mix(Color(0.04), baseColor, m);
    const Color F = schlick(F0, VoH);

    // --- G: Smith GGX Geometry---
    auto G1 = [&](const double NdotV) -> double
    {
        double safeDot = std::max(NdotV, 1e-7); // guard against grazing fp error
        double a2      = a * a;
        double NdotV2  = safeDot * safeDot;
        return 2.0 * safeDot / (safeDot + std::sqrt(a2 + (1.0 - a2) * NdotV2));
    };
    const double G = G1(NoV) * G1(NoL);

    // Typical Cook-Torrance approximation: D - normal distribution function (aka microfacets)
    // G - Geometry term, is used for masking microfacets
    // F - Fresnel term, is used to describe the amount of light that is reflected
    // N dot L - light from other direction excluded
    const Color specular = (D * G * F) / (4.0 * NoV * NoL + 1e-7);
    // Diffuse term is very similar to lambert, but metals lack it
    const Color diffuse = (1.0 - m) * baseColor / constants::kPi;

    const Color ao_value = ambient_occlusion(rec);
    

    return ao_value * diffuse + specular;

}
