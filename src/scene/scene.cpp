// Scene implementation using brute-force traversal over all primitives.

#include "scene/scene.h"
#include "geometry/mesh.h"
#include <iostream>

void Scene::loadSkyBox(  std::shared_ptr<Skybox> _skybox)
{
    this->skybox = std::move(_skybox);
}

void Scene::addPrimitive(const std::shared_ptr<Primitive>& primitive) {
    m_primitives.push_back(primitive);
}

void Scene::addLight(const std::shared_ptr<Light>& light) {
    m_lights.push_back(light);
}

bool Scene::intersect(const Ray& ray, HitRecord& rec) const
{
    if (bvh)
    {
        return bvh->intersect(ray, rec);
    }
    // Fallback: brute-force (useful before BVH is built / for debugging)
    bool hitAnything = false;
    Ray  closestRay  = ray;
    for (const auto& primitive : m_primitives) {
        if (primitive->intersect(closestRay, rec)) {
            hitAnything      = true;
            closestRay.tMax  = rec.t;
        }
    }
    return hitAnything;
}
bool Scene::occluded(const Ray& ray) const {
    HitRecord rec;
    return intersect(ray, rec);
}

const std::vector<std::shared_ptr<Light>>& Scene::lights() const {
    return m_lights;
}

void Scene::buildBVH()
{
    bvh = std::make_unique<BVH>();
    bvh->build(this->m_primitives);
}
