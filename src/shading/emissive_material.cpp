//
// Created by Asus on 6/1/2026.
//


#include "core/color.h"
#include "shading/emissive_material.h"

Color EmissiveMaterial::evaluate(const HitRecord& rec,
                                  const glm::dvec3& wo,
                                  const glm::dvec3& wi) const
{
    return Color(0.0);   // emissive surfaces don't scatter — no BRDF contribution
}

MaterialSample EmissiveMaterial::sample(const HitRecord& rec,
                                         const glm::dvec3& wo) const
{
    return MaterialSample{};   // valid = false — integrator sees no scatter, queries emitted()
}

double EmissiveMaterial::pdf(const HitRecord& rec,
                              const glm::dvec3& wo,
                              const glm::dvec3& wi) const
{
    return 0.0;
}

bool EmissiveMaterial::isDelta() const
{
    return false;
}

Color EmissiveMaterial::albedo(const HitRecord& rec) const
{
    return m_color;
}

