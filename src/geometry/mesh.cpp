//
// Created by Asus on 4/28/2026.
//
#include "geometry/mesh.h"

#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <ostream>
#include <vector>

#include "core/constants.h"
Mesh::Mesh(vector<Triangle> triangles, std::shared_ptr<Material> material)
    : triangles(std::move(triangles)), m_material((std::move(material)))
{
}

Mesh::Mesh(vector<Triangle> triangles,
    std::shared_ptr<Material> material,
    const vec3 &translation,
    const quat& rotation,
    const vec3& scale)
: triangles(std::move(triangles)), m_material((std::move(material))), translation(translation), scale(scale), rotation(rotation)
{
    for (auto & triangle : this->triangles)
    {
        triangle.set_translation(translation);
        triangle.set_rotation(rotation);
        triangle.set_scale(scale);
    }

}

// The least efficient traversal in the wild west
bool Mesh::intersect(const Ray& ray, HitRecord& rec) const
{
    bool intersects = false;
    double closest = ray.tMax; // ← use ray.tMax, not rec.t

    for (int i = 0; i < triangles.size(); ++i)
    {
        HitRecord tempRec;
        tempRec.t = closest;

        if (triangles[i].intersect(ray, tempRec) && tempRec.t < closest)
        {
            closest = tempRec.t;
            rec = tempRec;
            intersects = true;
        }
    }

    return intersects;
}

