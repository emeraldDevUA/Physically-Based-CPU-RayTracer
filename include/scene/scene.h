// Scene stores the current primitives and lights and answers ray queries.

#pragma once

#include <memory>
#include <vector>

#include "skybox.h"
#include "accel/bvh.h"
#include "geometry/primitive.h"
#include "shading/light.h"

class Scene {
public:
    void addPrimitive(const std::shared_ptr<Primitive>& primitive);
    void addLight(const std::shared_ptr<Light>& light);
    void loadSkyBox(std::shared_ptr<Skybox> skybox);

    void buildBVH();

    // Find the closest hit, if any.
    bool intersect(const Ray& ray, HitRecord& rec) const;

    // Return true as soon as any primitive blocks the ray.
    bool occluded(const Ray& ray) const;

    const std::vector<std::shared_ptr<Light>>& lights() const;

    std::shared_ptr<Skybox> getSkyBox() const{return skybox;};

private:
    std::vector<std::shared_ptr<Primitive>> m_primitives;
    std::vector<std::shared_ptr<Light>> m_lights;

    std::shared_ptr<Skybox> skybox;
    // Later directions may cache acceleration structures here.

    std::unique_ptr<BVH> bvh;
};
