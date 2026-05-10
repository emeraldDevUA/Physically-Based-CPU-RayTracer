// Scene implementation using brute-force traversal over all primitives.

#include "scene/scene.h"
#include "geometry/mesh.h"
#include <iostream>

void Scene::addPrimitive(const std::shared_ptr<Primitive>& primitive) {
    m_primitives.push_back(primitive);
}

void Scene::addLight(const std::shared_ptr<Light>& light) {
    m_lights.push_back(light);
}

bool Scene::intersect(const Ray& ray, HitRecord& rec) const {
    bool hitAnything = false;
    Ray closestRay = ray; // copy so we can shrink tMax

    for (const auto& primitive : m_primitives) {
         if (primitive->intersect(closestRay, rec)) {
              hitAnything = true;
              closestRay.tMax = rec.t; // only accept closer hits from now on
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