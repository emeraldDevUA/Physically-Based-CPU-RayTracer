// Starter stub for OBJ loading.
#define TINYOBJLOADER_IMPLEMENTATION  // ← must be here, before the include
#include "external-dependencies/tiny_obj_loader.h"
#include "io/obj_loader.h"

#include <iostream>
#include <memory>
#include "geometry/mesh.h"
#include "scene/scene.h"

bool ObjLoader::load(const std::string& path, Scene& scene, const std::shared_ptr<Material>& material,
const vec3& translation, const vec3& scale, const quat& rotation)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!ok)
    {
        std::cerr << "TinyObjLoader error: " << err << "\n";
        if (!warn.empty())
            std::cerr << "TinyObjLoader warning: " << warn << "\n";
        return false;
    }

    // Default material — replace with your actual Material subclass

    for (const auto& shape : shapes)
    {
        std::vector<Triangle> triangles;
        const auto& mesh = shape.mesh;
        size_t indexOffset = 0;

        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f)
        {
            // OBJ faces can be quads or polygons, but after triangulation they are always 3
            assert(mesh.num_face_vertices[f] == 3 && "Only triangulated meshes are supported");

            auto getVertex = [&](const int corner) -> glm::dvec3
            {
                const tinyobj::index_t idx = mesh.indices[indexOffset + corner];
                return {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };
            };

            auto getUV = [&](const int corner) -> glm::dvec2
            {
                const tinyobj::index_t idx = mesh.indices[indexOffset + corner];

                if (idx.texcoord_index >= 0) {
                    return {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    };
                }

                return {-0.1, -0.1};
            };

            glm::dvec3 a = getVertex(0);
            glm::dvec3 b = getVertex(1);
            glm::dvec3 c = getVertex(2);

            glm::dvec2 uvA = getUV(0);
            glm::dvec2 uvB = getUV(1);
            glm::dvec2 uvC = getUV(2);

            if (uvA.x < 0 || uvB.x < 0 || uvC.x < 0)
            {
                triangles.emplace_back(a, b, c, material);
            } else
            {
                triangles.emplace_back(a, b, c, uvA, uvB, uvC, material);
            }
            
            indexOffset += 3;
        }

        scene.addPrimitive(std::make_shared<Mesh>(std::move(triangles), material, translation, rotation, scale));
        
    }
    std::cout << "Loaded " << shapes.size() << " shapes\n";
    for (const auto& shape : shapes)
        std::cout << shape.name << ": " << shape.mesh.num_face_vertices.size() << " faces\n";
    return true;
}
