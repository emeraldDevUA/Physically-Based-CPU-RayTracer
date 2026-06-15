//
// Created by Asus on 5/16/2026.
//

#include <glm/glm.hpp>

#include "shading/material.h"
#include "core/constants.h"
#include "core/random.h"

// Builds an arbitrary but consistent tangent frame from just a normal.
// Uses Frisvad / Hughes-Möller to pick a stable perpendicular vector.
static void buildTBN(const glm::dvec3& N,
                     glm::dvec3& T,
                     glm::dvec3& B)
{
    // Frisvad's method — numerically stable except near (0, 0, -1)
    if (N.z < -0.9999999)
    {
        T = glm::dvec3(0.0, -1.0, 0.0);
        B = glm::dvec3(-1.0, 0.0, 0.0);
        return;
    }

    const double a = 1.0 / (1.0 + N.z);
    const double b = -N.x * N.y * a;

    T = glm::dvec3(1.0 - N.x * N.x * a, b, -N.x);
    B = glm::dvec3(b, 1.0 - N.y * N.y * a, -N.y);
}


GGXMaterial::GGXMaterial(const std::shared_ptr<Texture>& albedo)
{
    this->m_params.albedo = albedo;
}

GGXMaterial::GGXMaterial(Params params) : m_params(std::move(params))
{
}

Color GGXMaterial::albedo(const HitRecord& rec) const
{
    return this->m_params.albedo->value(rec.uv, rec.position);
}

double GGXMaterial::roughness(const HitRecord& rec) const
{
    if (!m_params.roughness)
    {
        return 0.5;
    }

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
    {
        return Color(0.0);
    }

    return this->m_params.emission->value(rec.uv, rec.position);
}

glm::dvec3 GGXMaterial::normal(const HitRecord& rec) const
{
    if (!m_params.normal)
        return rec.geometricNormal; // no normal map — use geometric normal

    // Sample tangent-space normal and remap [0,1] → [-1,1]
    const Color raw = m_params.normal->value(rec.uv, rec.position);
    const glm::dvec3 n = glm::dvec3(raw.r, raw.g, raw.b) * 2.0 - 1.0;

    // Transform from tangent space to world space via TBN
    const glm::dvec3 N = glm::normalize(n);
    glm::dvec3 T, B;
    buildTBN(N, T, B);

    return glm::normalize(T * n.x + B * n.y + N * n.z);
}

double schlick(double cosTheta, double etaRatio)
{
    double r0 = (1.0 - etaRatio) / (1.0 + etaRatio);
    r0 *= r0;
    return r0 + (1.0 - r0) * std::pow(1.0 - cosTheta, 5.0);
}

MaterialSample GGXMaterial::sample(const HitRecord& rec, const glm::dvec3& wo) const
{
    const glm::dvec3 N = normal(rec);
    const Color alb = albedo(rec);
    const Color emi = emission(rec);

    // ── Transmission path — dark albedo acts as glass ─────────────────────
    double lum = 0.2126 * alb.r + 0.7152 * alb.g + 0.0722 * alb.b;
    if (lum < 0.05)
    {
        constexpr double ior = 1.5;

        bool frontFace = glm::dot(wo, N) > 0.0;
        double etaRatio = frontFace ? (1.0 / ior) : ior;
        glm::dvec3 n = frontFace ? N : -N;

        glm::dvec3 unitWo = glm::normalize(wo);
        double cosTheta = std::min(glm::dot(unitWo, n), 1.0);
        double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

        bool tir = etaRatio * sinTheta > 1.0;
        double reflectance = schlick(cosTheta, etaRatio);

        glm::dvec3 wi;
        if (tir || reflectance > randomDouble())
        {
            wi = glm::reflect(-unitWo, n);
        }
        else
        {
            wi = glm::refract(-unitWo, n, etaRatio);
        }

        // Blend toward tinted transmission — dark albedo tints the glass
        Color transmitColor = lum < 0.01
                                  ? Color(1.0) // fully clear
                                  : glm::mix(emi, alb, lum / 0.05); // slight tint

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

    glm::dvec3 T, B;
    buildTBN(N, T, B);

    double m_roughness = roughness(rec);
    const double a = m_roughness * m_roughness;
    const double theta = std::atan(a * std::sqrt(r1) / std::sqrt(1.0 - r1));
    const double phi = 2.0 * constants::kPi * r2;

    const glm::dvec3 localH = {
        std::sin(theta) * std::cos(phi),
        std::sin(theta) * std::sin(phi),
        std::cos(theta)
    };

    const glm::dvec3 H = glm::normalize(T * localH.x + B * localH.y + N * localH.z);
    const glm::dvec3 wi = glm::normalize(2.0 * glm::dot(wo, H) * H - wo);

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

    ms.weight = evaluate(rec, wo, wi) / p;
    ms.valid = true;
    return ms;
}

double GGXMaterial::pdf(const HitRecord& rec,
                        const glm::dvec3& wo,
                        const glm::dvec3& wi) const
{
    const glm::dvec3 N = glm::normalize(rec.geometricNormal);
    const glm::dvec3 H = glm::normalize(wo + wi); // half vector

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
                            const glm::dvec3& wo,
                            const glm::dvec3& wi) const
{
    const glm::dvec3 N = normal(rec);
    const glm::dvec3 H = glm::normalize(wo + wi); // half-vector

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
    if (denom < 1e-6) return Color(0.0); // prevent D blowing up
    const double D = a2 / (constants::kPi * denom * denom);

    // --- F: Fresnel-Schlick ---
    // Dialectrics use F0 = 0.04; metals use the albedo as F0
    const Color F0 = glm::mix(Color(0.04), baseColor, m);
    const Color F = F0 + (Color(1.0) - F0) * std::pow(1.0 - VoH, 5.0);

    // --- G: Smith GGX Geometry (Schlick-Beckmann) ---
    auto G1 = [&](const double NdotV) -> double
    {
        double k = (a + 1.0);
        k = (k * k) / 8.0;
        return NdotV / (NdotV * (1.0 - k) + k);
    };
    const double G = G1(NoV) * G1(NoL);

    // Typical Cook-Torrance approximation: D - normal distribution function (aka microfacets)
    // G - Geometry term, is used for masking microfacets
    // F - Fresnel term, is used to describe the amount of light that is reflected
    // N dot L - light from other direction excluded
    const Color specular = (D * G * F) / (4.0 * NoV * NoL + 1e-7);
    // Diffuse term is very similar to lambert, but metals lack it
    const Color diffuse = (1.0 - m) * baseColor / constants::kPi;

    return (diffuse + specular);
}
