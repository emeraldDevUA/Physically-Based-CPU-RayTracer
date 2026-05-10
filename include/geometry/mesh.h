//
// Created by Asus on 4/28/2026.
//


#pragma once
#include "primitive.h"
#include "triangle.h"
#include "glm/vec3.hpp"

//#include "glm" quat


#include <vector>



using std::vector;
using glm::vec3;
using glm::vec4;
using glm::quat;

class Mesh: public Primitive
{
public:
    Mesh(vector<Triangle> triangles,
         std::shared_ptr<Material> material);

    Mesh(vector<Triangle> triangles,
        std::shared_ptr<Material> material,
        const vec3 &translation,
        const glm::quat& rotation,
        const vec3& scale);

    bool intersect(const Ray& ray, HitRecord& rec) const override;

private:
    // acceleration structure?
    vector<Triangle> triangles;
    std::shared_ptr<Material> m_material;

    vec3 translation{0, 0, 0};
    vec3 scale{1,1,1};
    quat rotation{1, 0, 0, 0}; // should be a quat, but some odd version of glm is used here...
};
