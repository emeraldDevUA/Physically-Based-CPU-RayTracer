// Triangle intersection implementation using the Moller-Trumbore algorithm.

#include "geometry/triangle.h"

#include <glm/glm.hpp>
#include <cmath>

#include "core/constants.h"

Triangle::Triangle(const glm::dvec3& a,
                   const glm::dvec3& b,
                   const glm::dvec3& c,
                   std::shared_ptr<Material> material)
    : m_a(a), m_b(b), m_c(c), m_material(std::move(material)) {}

bool Triangle::intersect(const Ray& ray, HitRecord& rec) const {
    const glm::dvec3 edge1 = m_b - m_a;
    const glm::dvec3 edge2 = m_c - m_a;

    const glm::dvec3 p_vec = glm::cross(ray.direction, edge2);
    const double det = glm::dot(edge1, p_vec);

    if (std::abs(det) < constants::kEpsilon) {
        return false;
    }

    const double invDet = 1.0 / det;
    const glm::dvec3 t_vec = ray.origin - m_a;

    const double u = glm::dot(t_vec, p_vec) * invDet;
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    const glm::dvec3 q_vec = glm::cross(t_vec, edge1);
    const double v = glm::dot(ray.direction, q_vec) * invDet;
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    const double t = glm::dot(edge2, q_vec) * invDet;
    if (t < ray.tMin || t > ray.tMax) {
        return false;
    }

    rec.t = t;
    rec.position = ray.at(t);

    const glm::dvec3 outwardNormal = glm::normalize(glm::cross(edge1, edge2));
    rec.setFaceNormal(ray.direction, outwardNormal);
    rec.material = m_material;
    rec.uv = glm::dvec2(u, v);

    return true;
}
