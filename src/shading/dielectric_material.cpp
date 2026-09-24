//
// Created by Asus on 5/24/2026.
//


#include "shading/dielectric_material.h"
#include "core/random.h"

// ── isDelta ───────────────────────────────────────────────────────────────────
// Glass is a delta distribution (perfect refract/reflect) — skip NEE/MIS.
bool DielectricMaterial::isDelta() const
{
    return true;
}

// ── pdf ───────────────────────────────────────────────────────────────────────
// Delta BRDFs have no finite density at any specific wi.
double DielectricMaterial::pdf(const HitRecord&,
                               const dvec3&,
                               const dvec3&) const
{
    return 0.0;
}

// ── evaluate ──────────────────────────────────────────────────────────────────
// Same reasoning as Mirror — the delta cannot be represented as a finite
// value at an arbitrary wi, so always return black.
Color DielectricMaterial::evaluate(const HitRecord&,
                                   const dvec3&,
                                   const dvec3&) const
{
    return Color(0.0);
}

// ── albedo ────────────────────────────────────────────────────────────────────
Color DielectricMaterial::albedo(const HitRecord&) const
{
    return Color(1.0);
    // glass transmits all channels equally
}

// ── sample ───────────────────────────────────────────────────────────────────
MaterialSample DielectricMaterial::sample(const HitRecord& rec,
                                          const dvec3& wo) const
{
    MaterialSample s;

    // Front face = ray coming from outside (wo points away from surface)
    bool frontFace = glm::dot(wo, rec.geometricNormal) > 0.0;
    double eta = frontFace ? (1.0 / m_ior) : m_ior;

    dvec3 n = frontFace ? rec.geometricNormal : -rec.geometricNormal;

    dvec3 wiIncident = -normalize(wo); // points into surface

    double cosTheta = (glm::dot(wiIncident, n));
    double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

    bool tir = eta * sinTheta > 1.0;
    double F = schlick(cosTheta, eta);

    dvec3 wi;
    if (tir || randomFloat() < F)
        wi = glm::reflect(wiIncident, n);
    else
        wi = glm::refract(wiIncident, n, eta);

    if (glm::dot(wi, wi) < 1e-12)
        return s; // cut invalid directions

    // Beer–Lambert absorption
    Color weight(1.0);
    if (!frontFace && m_absorptionDensity > 0.0)
    {
        weight = glm::exp(-m_absorptionDensity * rec.t * (Color(1.0) - m_absorptionColor));
    }

    s.wi = wi;
    s.weight = weight;
    s.pdf = 1.0;
    s.valid = true;
    s.delta = true;

    return s;
}
