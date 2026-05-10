// Placeholder interface for future mesh loading work.

#pragma once

#include <string>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "shading/material.h"

using glm::vec3;
using glm::vec4;
using glm::quat;

class Scene;

class ObjLoader {
public:
    // Returns false in the starter until OBJ loading is implemented.
    static bool load(const std::string& path, Scene& scene, const std::shared_ptr<Material>& material,
        const vec3& translation, const vec3& scale, const quat& rotation);
};
