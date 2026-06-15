// Infinite plane intersection implementation.

#include "geometry/plane.h"

#include <glm/glm.hpp>
#include <cmath>

#include "core/constants.h"

Plane::Plane(const glm::dvec3& point,
             const glm::dvec3& normal,
             std::shared_ptr<Material> material)
    : m_point(point),
      m_normal(glm::normalize(normal)),
      m_material(std::move(material)) {}

bool Plane::intersect(const Ray& ray, HitRecord& rec) const {
    const double denom = glm::dot(m_normal, ray.direction);

    if (std::abs(denom) < constants::kEpsilon) {
        return false;
    }

    const double t = glm::dot(m_point - ray.origin, m_normal) / denom;
    if (t < ray.tMin || t > ray.tMax) {
        return false;
    }

    rec.t = t;
    rec.position = ray.at(t);
    rec.setFaceNormal(ray.direction, m_normal);
    rec.material = m_material;
    rec.uv = glm::dvec2(rec.position.x, rec.position.z);

    return true;
}

BBox Plane::bounds() const
{
    constexpr double kInf = std::numeric_limits<double>::max();

    // Find which axis the normal is most aligned to, keep that axis thin.
    glm::dvec3 absN = glm::abs(m_normal);
    int dominantAxis = 0;
    if (absN.y > absN[dominantAxis]) dominantAxis = 1;
    if (absN.z > absN[dominantAxis]) dominantAxis = 2;

    glm::dvec3 minCorner(-kInf);
    glm::dvec3 maxCorner( kInf);

    // Thin slab on the dominant axis at the plane's offset position.
    const double d = glm::dot(m_normal, m_point);
    constexpr double kEps = 1e-4;
    minCorner[dominantAxis] = d - kEps;
    maxCorner[dominantAxis] = d + kEps;

    return {minCorner, maxCorner};

}
