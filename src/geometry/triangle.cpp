// Triangle intersection implementation using the Moller-Trumbore algorithm.

#include "geometry/triangle.h"

#include <glm/glm.hpp>
#include <cmath>
#include <glm/mat4x4.hpp>
#include "core/constants.h"

Triangle::Triangle(const glm::dvec3& a,
                   const glm::dvec3& b,
                   const glm::dvec3& c,
                   std::shared_ptr<Material> material)
    : m_a(a), m_b(b), m_c(c), m_material(std::move(material))
{
    m_at = m_a;
    m_bt = m_b;
    m_ct = m_c;
    uvs_assigned = false;
}


Triangle::Triangle(const glm::dvec3& a,
                   const glm::dvec3& b,
                   const glm::dvec3& c,
                   const glm::dvec2& uv_a,
                   const glm::dvec2& uv_b,
                   const glm::dvec2& uv_c,
                   std::shared_ptr<Material> material)
    : m_a(a), m_b(b), m_c(c), m_uv_a(uv_a), m_uv_b(uv_b), m_uv_c(uv_c), m_material(std::move(material))
{

    uvs_assigned = true;

    update_transformation();
}

void Triangle::update_transformation()
{
    auto q = glm::quat(rotation); // vec4 (w, x, y, z) — check GLM's constructor order

    m_at = glm::vec3(q * (glm::vec3(m_a) * scale)) + translation;
    m_bt = glm::vec3(q * (glm::vec3(m_b) * scale)) + translation;
    m_ct = glm::vec3(q * (glm::vec3(m_c) * scale)) + translation;
}

void Triangle::set_translation(const glm::dvec3& _translation)
{
    this->translation = _translation;

    update_transformation();
}
void Triangle::set_scale(const glm::dvec3& _scale)
{
    this->scale = _scale;

    update_transformation();
}
void Triangle::set_rotation(const glm::dquat& _rotation)
{
    this->rotation = _rotation;


    update_transformation();
}
bool Triangle::intersect(const Ray& ray, HitRecord& rec) const {


    const glm::dvec3 edge1 = m_bt - m_at;
    const glm::dvec3 edge2 = m_ct - m_at;

    const glm::dvec3 p_vec = glm::cross(ray.direction, edge2);
    const double det = glm::dot(edge1, p_vec);

    if (std::abs(det) < constants::kEpsilon) {
        return false;
    }

    const double invDet = 1.0 / det;
    const glm::dvec3 t_vec = ray.origin - m_at;

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

    if (!uvs_assigned)
    {
        rec.uv = glm::dvec2(u, v);
    }else {
        double w = 1.0 - u - v;
        rec.uv = w * m_uv_a + u * m_uv_b + v * m_uv_c;
    }

    return true;
}
