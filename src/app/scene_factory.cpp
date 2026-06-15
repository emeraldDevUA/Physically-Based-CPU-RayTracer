// Implementation of reusable scene presets for the starter project.

#include "app/scene_factory.h"

#include <iostream>
#include <memory>

#include "geometry/geometry.h"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "io/obj_loader.h"
#include "shading/dielectric_material.h"
#include "shading/emissive_material.h"
#include "shading/mirror_material.h"
#include "shading/shading.h"

SceneSetup SceneFactory::createStarterScene(const int width, const int height) {
    Scene scene;

    // ── Materials ─────────────────────────────────────────────────────────────
    auto green  = std::make_shared<LambertMaterial>(Color(0.2, 0.8, 0.2));
    auto mirror = std::make_shared<MirrorMaterial>(vec3(0.8, 0.2, 0.2));
    auto glass  = std::make_shared<DielectricMaterial>(1.5);

    auto tiled_tex = std::make_shared<ImageTexture>("../../assets/textures/painted_concrete_diff_1k.png");
    auto gray      = std::make_shared<LambertMaterial>(tiled_tex);

    // ── GGX Crystal ───────────────────────────────────────────────────────────
    auto makeTex = [](const char* path) {
        return std::make_shared<ImageTexture>(path);
    };

    GGXMaterial::Params crystalParams;
    crystalParams.albedo    = makeTex("../../assets/models/ggx-model/crystal_albedo.png");
    crystalParams.roughness = makeTex("../../assets/models/ggx-model/crystal_roughness.png");
    crystalParams.metallic  = makeTex("../../assets/models/ggx-model/crystal_metallic.png");
    crystalParams.normal    = makeTex("../../assets/models/ggx-model/crystal_normal.png");
    crystalParams.emission  = makeTex("../../assets/models/ggx-model/crystal_emissive.png");
    auto crystalMat = std::make_shared<GGXMaterial>(crystalParams);

    ObjLoader::load("../../assets/models/ggx-model/crystal.obj", scene, crystalMat, vec3(-1.3, -0.5, -1.5), vec3(0.028), glm::angleAxis(glm::radians(15.0f), vec3(0.0f, 1.0f, 0.0f)));

    // ── GGX Potion ────────────────────────────────────────────────────────────
    GGXMaterial::Params potionParams;
    potionParams.albedo    = makeTex("../../assets/models/potion/flask_baseColor.png");
    potionParams.roughness = makeTex("../../assets/models/potion/flask_metallicRoughness.png");
    potionParams.metallic  = makeTex("../../assets/models/potion/flask_metallicRoughness.png");
    potionParams.normal    = makeTex("../../assets/models/potion/flask_normal.png");
    auto potionMat = std::make_shared<GGXMaterial>(potionParams);

    ObjLoader::load(
        "../../assets/models/potion/potion_flask.obj", scene, potionMat,
        vec3(1.5, -1.0, -1.5), vec3(0.7),
        glm::angleAxis(glm::radians(0.0f), vec3(0.0f, 1.0f, 0.0f)));

    // ── Spheres ───────────────────────────────────────────────────────────────
    // Glass sphere — center stage, slightly closer
    scene.addPrimitive(std::make_shared<Sphere>( glm::dvec3(0.5, -0.35, -2.9), 0.65, glass));
    scene.addPrimitive(std::make_shared<Sphere>( glm::dvec3(-0.35, -0.10, -4.4), 0.9, mirror));
    scene.addPrimitive(std::make_shared<Sphere>( glm::dvec3(1.35, -0.35, -5.9), 0.65, green));
    // ── Floor ─────────────────────────────────────────────────────────────────
    scene.addPrimitive(std::make_shared<Triangle>( glm::dvec3(-8.0, -1.0, 4.0), glm::dvec3( 8.0, -1.0, 4.0), glm::dvec3( 8.0, -1.0, -12.0), gray)); scene.addPrimitive(std::make_shared<Triangle>( glm::dvec3(-8.0, -1.0, 4.0), glm::dvec3( 8.0, -1.0, -12.0), glm::dvec3(-8.0, -1.0, -12.0), gray));

    // ── Lighting ──────────────────────────────────────────────────────────────
    // Key light — right, warm
    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(4.0, 5.0, 1.0),
        Color(1.0, 0.95, 0.88), 35.0));

    // Fill light — left, cooler
    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(-4.0, 4.0, 1.0),
        Color(0.88, 0.92, 1.0), 20.0));

    // Rim/back light — behind scene, neutral
    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(0.0, 6.0, -6.0),
        Color(1.0, 1.0, 1.0), 18.0));

    // ── Skybox & BVH ─────────────────────────────────────────────────────────
    scene.loadSkyBox(std::make_shared<Skybox>("../../assets/skybox"));
    scene.buildBVH();

    // ── Camera ────────────────────────────────────────────────────────────────
    const Camera camera(
        glm::dvec3(-0.6, 1.3, 2.4),    // eye — slightly higher for better angle
        glm::dvec3(0.2, 0.2, -2.5),   // target — looks into the scene
        glm::dvec3(0.0, 1.0,  0.0),   // up
        42.0,                          // slightly narrower FOV to tighten framing
        width,
        height
    );

    return { std::move(scene), camera };
}


SceneSetup SceneFactory::createDielectricScene(const int width, const int height)
{
    Scene scene;

    // --------------------------------------------------
    // Helpers
    // --------------------------------------------------
    auto createGlass = [](
        double ior,
        const Color& tint,
        double density)
    {
        return std::make_shared<DielectricMaterial>(
            ior,
            tint,
            density);
    };

    auto loadModel = [&](const char* path,
                         const std::shared_ptr<Material>& material,
                         const vec3& position,
                         const vec3& scale,
                         float rotationY)
    {
        ObjLoader::load(
            path,
            scene,
            material,
            position,
            scale,
            glm::angleAxis(
                glm::radians(rotationY),
                vec3(0.0f, 1.0f, 0.0f))
        );
    };

    auto addSphere = [&](const glm::dvec3& position,
                         double radius,
                         const std::shared_ptr<Material>& material)
    {
        scene.addPrimitive(
            std::make_shared<Sphere>(
                position,
                radius,
                material));
    };

    // --------------------------------------------------
    // Dielectric materials
    // --------------------------------------------------
    auto greenGlass = createGlass(
        1.5,
        Color(0.2, 0.9, 0.3),
        0.15);

    auto amberGlass = createGlass(
        1.5,
        Color(0.9, 0.6, 0.1),
        0.2);

    auto deepBlue = createGlass(
        1.5,
        Color(0.1, 0.3, 0.9),
        0.3);

    auto rubyGlass = createGlass(
        1.7,
        Color(0.9, 0.05, 0.05),
        0.25);

    // --------------------------------------------------
    // Floor
    // --------------------------------------------------
    auto gray = std::make_shared<LambertMaterial>(
        Color(0.6, 0.6, 0.6));

    scene.addPrimitive(std::make_shared<Triangle>(
        glm::dvec3(-6.0, -1.0, -1.0),
        glm::dvec3(6.0, -1.0, -1.0),
        glm::dvec3(0.0, -1.0, -10.0),
        gray));

    scene.addPrimitive(std::make_shared<Triangle>(
        glm::dvec3(-6.0, -1.0, -1.0),
        glm::dvec3(0.0, -1.0, -10.0),
        glm::dvec3(-6.0, -1.0, -10.0),
        gray));

    // --------------------------------------------------
    // Glass spheres
    // --------------------------------------------------
    // addSphere(
    //     glm::dvec3(-3.0, -0.2, -4.0),
    //     0.8,
    //     greenGlass);

    addSphere(
        glm::dvec3(-1.0, -0.2, -4.0),
        0.8,
        amberGlass);

    addSphere(
        glm::dvec3(1.0, -0.2, -4.0),
        0.8,
        deepBlue);

    addSphere(
        glm::dvec3(3.0, -0.2, -4.0),
        0.8,
        rubyGlass);

    // --------------------------------------------------
    // Models
    // --------------------------------------------------
    loadModel(
        "../../assets/models/glass/glass.obj",
        deepBlue,
        glm::dvec3(-3.0, -0.2, -4.0),
        vec3(0.8),
        25.0f);

    loadModel(
        "../../assets/models/cube/cube.obj",
        rubyGlass,
        vec3(0.0, -1.0, -6.5),
        vec3(0.8),
        -15.0f);

    loadModel(
        "../../assets/models/cube/cube.obj",
        deepBlue,
        vec3(2.5, -1.0, -6.5),
        vec3(0.8),
        40.0f);

    // --------------------------------------------------
    // Reference spheres
    // --------------------------------------------------
    addSphere(
        glm::dvec3(-2.5, 0.2, -6.5),
        0.05,
        rubyGlass);

    addSphere(
        glm::dvec3(0.0, 0.2, -6.5),
        0.03,
        greenGlass);

    addSphere(
        glm::dvec3(2.5, 0.2, -6.5),
        0.02,
        greenGlass);

    // --------------------------------------------------
    // Lights
    // --------------------------------------------------
    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(4.0, 5.0, 0.0),
        Color(1.0, 1.0, 1.0),
        30.0));

    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(-4.0, 5.0, 0.0),
        Color(0.8, 0.9, 1.0),
        20.0));

    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(0.0, 3.0, 1.0),
        Color(1.0, 0.95, 0.8),
        10.0));

    // --------------------------------------------------
    // Environment
    // --------------------------------------------------
    auto skybox = std::make_shared<Skybox>(
        "../../assets/skybox2");

    scene.loadSkyBox(skybox);

    scene.buildBVH();

    // --------------------------------------------------
    // Camera
    // --------------------------------------------------
    const Camera camera(
        glm::dvec3(0.0, 4, 3.0),
        glm::dvec3(0.0, 0.0, -4.5),
        glm::dvec3(0.0, 1.0, 0.0),
        50.0,
        width,
        height
    );

    return { std::move(scene), camera };
}


SceneSetup SceneFactory::createAircraftScene(const int width, const int height)
{
    Scene scene;

    // --------------------------------------------------
    // Helpers
    // --------------------------------------------------
    auto createGlass = [](
        double ior,
        const Color& tint,
        double density)
    {
        return std::make_shared<DielectricMaterial>(
            ior,
            tint,
            density);
    };

    auto loadModel = [&](const char* path,
                         const std::shared_ptr<Material>& material,
                         const vec3& position,
                         const vec3& scale,
                         float rotationY)
    {
        ObjLoader::load(
            path,
            scene,
            material,
            position,
            scale,
            glm::angleAxis(
                glm::radians(rotationY),
                vec3(0.0f, 1.0f, 0.0f))
        );
    };

    auto addSphere = [&](const glm::dvec3& position,
                         double radius,
                         const std::shared_ptr<Material>& material)
    {
        scene.addPrimitive(
            std::make_shared<Sphere>(
                position,
                radius,
                material));
    };

    // --------------------------------------------------
    // Dielectric materials
    // --------------------------------------------------
    auto greenGlass = createGlass(
        1.5,
        Color(0.2, 0.9, 0.3),
        0.15);


    auto makeTex = [](const char* path) {
        return std::make_shared<ImageTexture>(path);
    };

    GGXMaterial::Params planeParams;

    try
    {
        planeParams.albedo    = makeTex("../../assets/models/F16/F16_albedo.png");
        planeParams.roughness = makeTex("../../assets/models/F16/F16_roughness.png");
        planeParams.metallic  = makeTex("../../assets/models/F16/F16_metalness.png");
        planeParams.normal    = makeTex("../../assets/models/F16/F16_normal.png");
    } catch (std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
    auto planeMat = std::make_shared<GGXMaterial>(planeParams);

    // --------------------------------------------------
    // Floor
    // --------------------------------------------------
    auto gray = std::make_shared<LambertMaterial>(
        Color(0.6, 0.6, 0.6));

    scene.addPrimitive(std::make_shared<Triangle>(
        glm::dvec3(-6.0, -1.0, -1.0),
        glm::dvec3(6.0, -1.0, -1.0),
        glm::dvec3(0.0, -1.0, -10.0),
        gray));

    scene.addPrimitive(std::make_shared<Triangle>(
        glm::dvec3(-6.0, -1.0, -1.0),
        glm::dvec3(0.0, -1.0, -10.0),
        glm::dvec3(-6.0, -1.0, -10.0),
        gray));

    // --------------------------------------------------
    // Glass spheres
    // --------------------------------------------------
    // addSphere(
    //     glm::dvec3(-3.0, -0.2, -4.0),
    //     0.8,
    //     greenGlass);


    // --------------------------------------------------
    // Models
    // --------------------------------------------------
    loadModel(
        "../../assets/models/F16/canopy.obj",
        greenGlass,
        glm::dvec3(-0.0, 0.0, -2.5),
        vec3(1.4),
        -75);

    loadModel(
        "../../assets/models/F16/f16.obj",
        planeMat,
        glm::dvec3(-0.0, 0.0, -2.5),
        vec3(1.4),
        -75);


    // --------------------------------------------------
    // Lights
    // --------------------------------------------------
    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(4.0, 5.0, 0.0),
        Color(1.0, 1.0, 1.0),
        30.0));

    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(-4.0, 5.0, 0.0),
        Color(0.8, 0.9, 1.0),
        20.0));

    scene.addLight(std::make_shared<PointLight>(
        glm::dvec3(0.0, 3.0, 1.0),
        Color(1.0, 0.95, 0.8),
        10.0));

    // --------------------------------------------------
    // Environment
    // --------------------------------------------------
    auto skybox = std::make_shared<Skybox>(
        "../../assets/skybox2");

    scene.loadSkyBox(skybox);

    scene.buildBVH();

    // --------------------------------------------------
    // Camera
    // --------------------------------------------------
    const Camera camera(
        glm::dvec3(-0.8, 1.1, 1.6),
        glm::dvec3(0.8, 0.5, -4.5),
        glm::dvec3(0.0, 1.0, 0.0),
        50.0,
        width,
        height
    );

    return { std::move(scene), camera };
}