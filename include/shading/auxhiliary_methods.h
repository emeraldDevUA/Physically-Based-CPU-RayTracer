//
// Created by Asus on 6/19/2026.
//

#pragma once
#include "core/color.h"
#include "core/constants.h"
#include "glm/vec3.hpp"

using glm::dvec3;

// Builds an arbitrary but consistent tangent frame from just a normal.
// Uses Frisvad / Hughes-Möller to pick a stable perpendicular vector.
inline void buildTBN(const dvec3& N,
                     dvec3& T,
                     dvec3& B)
{
    // Frisvad's method — numerically stable except near (0, 0, -1)
    if (N.z < - (1 - constants::kEpsilon))
    {
        T = dvec3(0.0, -1.0, 0.0);
        B = dvec3(-1.0, 0.0, 0.0);
        return;
    }

    const double a = 1.0 / (1.0 + N.z);
    const double b = -N.x * N.y * a;

    T = dvec3(1.0 - N.x * N.x * a, b, -N.x);
    B = dvec3(b, 1.0 - N.y * N.y * a, -N.y);
}

inline double schlick(const double cosTheta, const double etaRatio)
{
    double r0 = (1.0 - etaRatio) / (1.0 + etaRatio);
    r0 *= r0;
    return r0 + (1.0 - r0) * std::pow(1.0 - cosTheta, 5.0);
}

inline Color schlick(const Color& F0, const double VoH)
{
    return F0 + (Color(1.0) - F0) * std::pow(1.0 - VoH, 5.0);
}

inline double toGrayscale(const Color& color)
{
    return  0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}