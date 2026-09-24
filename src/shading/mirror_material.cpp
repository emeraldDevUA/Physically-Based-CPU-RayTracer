//
// Created by Asus on 5/24/2026.
//
#include "shading/mirror_material.h"

#include "core/constants.h"

Color MirrorMaterial::albedo(const HitRecord& rec) const
{

    return {0.0, 0.0, 0.0};
}

bool MirrorMaterial::isDelta() const {
    return true;
}

double MirrorMaterial::pdf(const HitRecord& /*rec*/,
                   const dvec3& /*wo*/,
                   const dvec3& /*wi*/) const
{

    // is equal to zero, because of the perfect specular scattering
    return 0.0;
}

MaterialSample MirrorMaterial::sample(const HitRecord& rec,
                               const dvec3& wo) const
{
    MaterialSample s;                          // valid = false, delta = false by default

    const dvec3 wi = glm::reflect(-wo, rec.geometricNormal);

    // Geometric normal check — kills paths that tunnel through the mesh
    // when shading normals (from normal maps) diverge from geometry.
    if (glm::dot(wi, rec.geometricNormal) <= 0.0)
        return s;                              // valid = false → integrator discards

    // The delta BRDF is  f = R * δ(wi − wr) / cosθi.
    // The rendering equation integral collapses the delta and cosθi cancels,
    // leaving weight = R.  pdf = 1 so the integrator multiplies throughput
    // by weight / pdf = R exactly, with no extra cosine term.
    s.wi    = wi;
    s.weight = this->m_reflectance;
    s.pdf   = 1.0;
    s.valid = true;
    s.delta = true;

    return s;
}

Color MirrorMaterial::evaluate(const HitRecord& rec,
                       const dvec3& wo,
                       const dvec3& wi) const
{
    // Reconstruct the exact mirror direction for this wo.
    const dvec3 wiPerfect = glm::reflect(-wo, rec.geometricNormal);

    // Only return a non-zero value if wi matches to within floating-point
    // precision. In a correct integrator this branch is never taken from
    // MIS/NEE paths — it exists only as a safety valve.
    if (glm::dot(wi, wiPerfect) < 1.0 - constants::kEpsilon)
        return Color(0.0);

    // The cos(θi) in the rendering equation cancels the 1/cos(θi) factor
    // of the delta BRDF, so we return just the reflectance.
    return m_reflectance;
}