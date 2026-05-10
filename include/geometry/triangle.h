// Triangle primitive for the minimal starter.
// This allows the project to move toward mesh rendering later.

#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "geometry/primitive.h"
#include "glm/detail/type_quat.hpp"
#include "shading/material.h"

class Triangle : public Primitive {
public:
    Triangle(const glm::dvec3& a,
             const glm::dvec3& b,
             const glm::dvec3& c,
             std::shared_ptr<Material> material);

    Triangle(const glm::dvec3& a,
         const glm::dvec3& b,
         const glm::dvec3& c,
         const glm::dvec2& uv_a,
         const glm::dvec2& uv_b,
         const glm::dvec2& uv_c,
         std::shared_ptr<Material> material);

    void update_transformation();

    bool intersect(const Ray& ray, HitRecord& rec) const override;


    void set_translation(const glm::dvec3& translation);
    void set_scale(const glm::dvec3& scale);
    void set_rotation(const glm::dquat& rotation);
private:
    glm::dvec3 m_a;
    glm::dvec3 m_b;
    glm::dvec3 m_c;

    // transformations cached for performance
    glm::dvec3 m_at;
    glm::dvec3 m_bt;
    glm::dvec3 m_ct;

    // uvs
    glm::dvec2 m_uv_a;
    glm::dvec2 m_uv_b;
    glm::dvec2 m_uv_c;

    bool uvs_assigned;

    glm::vec3 translation{0};
    glm::vec3 scale{1};
    glm::quat rotation{1,0,0,0}; // should be a quat, but some odd version of glm is used here...

    std::shared_ptr<Material> m_material;
};
