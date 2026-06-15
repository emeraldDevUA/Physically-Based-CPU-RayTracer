// Axis-aligned bounding box type reserved for later acceleration work.

#pragma once

#include <glm/glm.hpp>
#include "core/ray.h"
class BBox {
public:
    BBox();
    BBox(const glm::dvec3& minCorner, const glm::dvec3& maxCorner);

    const glm::dvec3& min() const { return m_min; }
    const glm::dvec3& max() const { return m_max; }

    glm::dvec3 centroid()    const { return (m_min + m_max) * 0.5; }
    double     surfaceArea() const;
    int        maxExtentAxis() const;

    bool intersect(const Ray& ray, double tMin, double tMax) const;

    static BBox unite(const BBox& a, const BBox& b);
    static BBox unite(const BBox& a, const glm::dvec3& p);

private:
    glm::dvec3 m_min;
    glm::dvec3 m_max;
};