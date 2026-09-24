//
// Created by Asus on 4/28/2026.
//

#include <glm/glm.hpp>
#include <vector>

#include "geometry/mesh.h"

Mesh::Mesh(vector<Triangle> triangles, std::shared_ptr<Material> material)
    : m_material(std::move(material))
{
    for (auto& t : triangles)
        this->triangles.push_back(std::make_shared<Triangle>(std::move(t)));
}

Mesh::Mesh(vector<Triangle> triangles,
           std::shared_ptr<Material> material,
           const vec3& translation,
           const quat& rotation,
           const vec3& scale)
    : m_material(std::move(material))
      ,translation(translation), scale(scale), rotation(rotation)
{
    for (auto& t : triangles)
    {
        t.set_translation(translation);
        t.set_rotation(rotation);
        t.set_scale(scale);
        this->triangles.push_back(std::make_shared<Triangle>(std::move(t)));
    }
    build();
}

bool Mesh::intersect(const Ray& ray, HitRecord& rec) const
{
    // Use BVH if built, brute-force fallback otherwise
    if (m_bvh)
        return m_bvh->intersect(ray, rec);

    bool hit = false;
    double closest = ray.tMax;
    for (const auto& tri : triangles)
    {
        HitRecord tmp;
        if (tri->intersect(ray, tmp) && tmp.t < closest)
        {
            closest = tmp.t;
            rec = tmp;
            hit = true;
        }
    }
    return hit;
}

BBox Mesh::bounds() const
{
    BBox b;
    for (const auto& tri : triangles)
        b = BBox::unite(b, tri->bounds());
    return b;
}

void Mesh::build()
{
    m_bvh = std::make_unique<BVH>();
    m_bvh->build(std::vector<std::shared_ptr<Primitive>>(
        triangles.begin(), triangles.end()));

    m_bvh->dumpWireframeObj("bvh_debug.obj");        // full tree
    m_bvh->dumpWireframeObj("bvh_leaves.obj", 3);    // just top 3 levels, for a coarse look
}
