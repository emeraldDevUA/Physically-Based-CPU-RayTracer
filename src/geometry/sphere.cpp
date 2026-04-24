// Sphere intersection implementation.

#include "geometry/sphere.h"

#include <glm/glm.hpp>
#include <cmath>

Sphere::Sphere(const glm::dvec3& center,
               const double radius,
               std::shared_ptr<Material> material)
    : m_center(center), m_radius(radius), m_material(std::move(material)) {}


bool Sphere::intersect(const Ray& ray, HitRecord& rec) const {
    const glm::dvec3 oc = ray.origin - m_center;

    const double a = glm::dot(ray.direction, ray.direction);
    const double h = glm::dot(oc, ray.direction);
    const double c = glm::dot(oc, oc) - m_radius * m_radius;

    const double discriminant = h * h - a * c;
    if (discriminant < 0.0) {
        return false;
    }

    const double sqrtDisc = std::sqrt(discriminant);
    const double invA = 1.0 / a;

    // Try nearer root first, then farther
    double t = (-h - sqrtDisc) * invA;
    if (t < ray.tMin || t > ray.tMax) {
        t = (-h + sqrtDisc) * invA;
        if (t < ray.tMin || t > ray.tMax) {
            return false;
        }
    }

    rec.t = t;
    rec.position = ray.at(t);

    const glm::dvec3 outwardNormal = (rec.position - m_center) / m_radius;
    rec.setFaceNormal(ray.direction, outwardNormal);
    rec.material = m_material;

    // Spherical UV mapping
    const glm::dvec3 n = outwardNormal;

    constexpr double pi_value = 3.14159265358979323846;

    const double theta = std::acos(-n.y);
    const double phi   = std::atan2(-n.z, n.x) + pi_value;
    rec.uv = glm::dvec2(phi / (2.0 * pi_value), theta / pi_value);

    return true;
}
