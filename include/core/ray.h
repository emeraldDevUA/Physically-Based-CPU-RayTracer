// Basic ray structure used for all ray-scene queries.
// The ray interval [tMin, tMax] helps avoid self-intersections.

#pragma once

#include <glm/glm.hpp>
#include "core/constants.h"
using glm::dvec3;

struct Ray {
    dvec3 origin{0.0, 0.0, 0.0};
    dvec3 direction{0.0, 0.0, -1.0};
    double tMin = constants::kEpsilon;
    double tMax = constants::kInfinity;

    Ray() = default;

    Ray(const dvec3& o,
        const dvec3& d,
        const double minT = constants::kEpsilon,
        const double maxT = constants::kInfinity)
        : origin(o), direction(d), tMin(minT), tMax(maxT) {}

    dvec3 at(const double t) const {
        return origin + t * direction;
    }
};