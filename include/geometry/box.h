//
// Created by Asus on 7/12/2026.
//

#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "geometry/primitive.h"
#include "shading/material.h"

// Oriented bounding box (a rectangular box that need not be axis-aligned).
// Orientation is stored as a unit quaternion rotating the box's local
// x/y/z axes into world space.
class Box : public Primitive {
public:
    Box(const dvec3& center,
        const glm::dquat& rotation,
        const dvec3& halfExtents,
        std::shared_ptr<Material> material);

    bool intersect(const Ray& ray, HitRecord& rec) const override;

    BBox bounds() const override;

private:
    dvec3 m_center;
    glm::dquat m_rotation;   // unit quaternion: local axes -> world space
    dvec3 m_halfExtents;     // half-widths along local x/y/z
    std::shared_ptr<Material> m_material;
};