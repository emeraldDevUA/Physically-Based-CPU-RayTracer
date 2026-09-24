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
    const auto q = glm::quat(rotation); // vec4 (w, x, y, z) — check GLM's constructor order

    m_at = vec3(q * (vec3(m_a) * scale)) + translation;
    m_bt = vec3(q * (vec3(m_b) * scale)) + translation;
    m_ct = vec3(q * (vec3(m_c) * scale)) + translation;
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

    // Two edge vectors of the triangle, both originating from vertex A.
    // Together with A these fully define the triangle's plane.
    const dvec3 edge1 = m_bt - m_at;
    const dvec3 edge2 = m_ct - m_at;

    // p_vec is perpendicular to both the ray direction and edge2 — it's an
    // intermediate quantity used by the Möller–Trumbore algorithm to solve
    // for the barycentric coordinate u and the determinant in one step.
    const dvec3 p_vec = glm::cross(ray.direction, edge2);

    // The determinant of the linear system [-, edge1, edge2] wrt the ray.
    // Geometrically, this is proportional to how parallel the ray is to
    // the triangle's plane: it approaches 0 as the ray direction becomes
    // parallel to the triangle (grazing/edge-on), where no unique
    // intersection point exists.
    const double det = glm::dot(edge1, p_vec);

    // Ray is (nearly) parallel to the triangle's plane — no valid hit.
    // Note: this also implicitly handles back-face culling only if det's
    // sign is checked elsewhere; here both signs of det are accepted, so
    // both triangle faces can be hit.
    if (std::abs(det) < constants::kEpsilon) {
        return false;
    }

    const double invDet = 1.0 / det;

    // Vector from vertex A to the ray origin — used to express the ray
    // origin's position relative to the triangle for the barycentric solve.
    const dvec3 t_vec = ray.origin - m_at;

    // First barycentric coordinate (weight of vertex B). If it falls
    // outside [0, 1], the hit point lies outside the triangle along this
    // axis, so we can reject early without computing v or t.
    const double u = glm::dot(t_vec, p_vec) * invDet;
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    // q_vec is the analogous intermediate quantity used to solve for the
    // second barycentric coordinate v and the ray parameter t.
    const dvec3 q_vec = glm::cross(t_vec, edge1);

    // Second barycentric coordinate (weight of vertex C). Combined with the
    // u check above, u + v > 1 means the point falls outside the triangle
    // on the far side from vertex A (the third barycentric weight,
    // 1 - u - v, would be negative).
    const double v = glm::dot(ray.direction, q_vec) * invDet;
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    // Ray parameter t at the intersection point (distance along the ray,
    // scaled by the ray direction's length). Reject hits outside the
    // ray's valid [tMin, tMax] range (e.g. behind the origin, or beyond
    // the closest hit found so far).
    const double t = glm::dot(edge2, q_vec) * invDet;
    if (t < ray.tMin || t > ray.tMax) {
        return false;
    }

    // Valid hit — record the intersection distance and world-space
    // position along the ray.
    rec.t = t;
    rec.position = ray.at(t);

    // Geometric normal of the triangle's plane (unnormalized cross product
    // of the two edges, then normalized). setFaceNormal presumably flips
    // this to face against the incoming ray direction and records whether
    // the hit was on the front or back face.
    const dvec3 outwardNormal = normalize(glm::cross(edge1, edge2));
    rec.setFaceNormal(ray.direction, outwardNormal);
    rec.material = m_material;

    // Texture coordinates: if the triangle has no explicit per-vertex UVs
    // assigned, fall back to the raw barycentric (u, v) as a cheap default
    // UV mapping. Otherwise, interpolate the triangle's actual per-vertex
    // UVs using the barycentric weights (w for A, u for B, v for C).
    if (!uvs_assigned)
    {
        rec.uv = glm::dvec2(u, v);
    } else {
        const double w = 1.0 - u - v;
        rec.uv = w * m_uv_a + u * m_uv_b + v * m_uv_c;
    }

    return true;
}

BBox Triangle::bounds() const
{
    dvec3 minCorner = glm::min(glm::min(m_at, m_bt), m_ct);
    dvec3 maxCorner = glm::max(glm::max(m_at, m_bt), m_ct);

    // Pad degenerate axis-aligned triangles (e.g. flat on XZ plane)
    // so the BBox has non-zero volume and the slab test doesn't miss.
    for (int i = 0; i < 3; ++i)
    {
        if (maxCorner[i] - minCorner[i] < constants::kEpsilon) {
            minCorner[i] -= constants::kEpsilon;
            maxCorner[i] += constants::kEpsilon;
        }
    }

    return {minCorner, maxCorner};
}

