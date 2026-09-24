// Implementation of reusable scene presets for the starter project.

#include "app/scene_factory.h"

#include <iostream>
#include <memory>

#include "geometry/box.h"
#include "geometry/geometry.h"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "io/obj_loader.h"
#include "shading/blinn_phong_material.h"
#include "shading/dielectric_material.h"
#include "shading/mirror_material.h"
#include "shading/shading.h"
#include "shading/ggx_material.h"

SceneSetup SceneFactory::createStarterScene(const int width, const int height)
{
    Scene scene;


    BlinnPhongMaterial::Params params;

    params.albedo = std::make_shared<ConstantTexture>(Color(0.2, 0.7, 0.2));
    params.normal = std::make_shared<ConstantTexture>(Color(0.5, 0.5, 1.0)); // flat normal map

    params.diffuse = std::make_shared<ConstantTexture>(Color(0.2, 0.7, 0.2));
    params.specular = std::make_shared<ConstantTexture>(Color(0.04, 0.04, 0.04)); // dielectric
    params.ambient = std::make_shared<ConstantTexture>(Color(0.02, 0.07, 0.02));

    // ── Materials ─────────────────────────────────────────────────────────────
    auto green = std::make_shared<BlinnPhongMaterial>(params);
    auto mirror = std::make_shared<MirrorMaterial>(vec3(0.8, 0.2, 0.2));
    auto glass = std::make_shared<DielectricMaterial>(1.5);

    auto tiled_tex = std::make_shared<ImageTexture>("../../assets/textures/painted_concrete_diff_1k.png");
    auto gray = std::make_shared<LambertMaterial>(tiled_tex);

    // ── GGX Crystal ───────────────────────────────────────────────────────────
    auto makeTex = [](const char* path)
    {
        return std::make_shared<ImageTexture>(path);
    };

    GGXMaterial::Params crystalParams;
    crystalParams.albedo = makeTex("../../assets/models/ggx-model/crystal_albedo.png");
    crystalParams.roughness = makeTex("../../assets/models/ggx-model/crystal_roughness.png");
    crystalParams.metallic = makeTex("../../assets/models/ggx-model/crystal_metallic.png");
    crystalParams.normal = makeTex("../../assets/models/ggx-model/crystal_normal.png");
    crystalParams.emission = makeTex("../../assets/models/ggx-model/crystal_emissive.png");
    auto crystalMat = std::make_shared<GGXMaterial>(crystalParams);

    ObjLoader::load("../../assets/models/ggx-model/crystal.obj", scene, crystalMat, vec3(-1.3, -0.5, -1.5), vec3(0.028),
                    glm::angleAxis(glm::radians(15.0f), vec3(0.0f, 1.0f, 0.0f)));

    // ── GGX Potion ────────────────────────────────────────────────────────────
    GGXMaterial::Params potionParams;
    potionParams.albedo = makeTex("../../assets/models/potion/flask_baseColor.png");
    potionParams.roughness = makeTex("../../assets/models/potion/flask_metallicRoughness.png");
    potionParams.metallic = makeTex("../../assets/models/potion/flask_metallicRoughness.png");
    potionParams.normal = makeTex("../../assets/models/potion/flask_normal.png");
    auto potionMat = std::make_shared<GGXMaterial>(potionParams);
    //
    ObjLoader::load(
        "../../assets/models/potion/potion_flask_lod.obj", scene, potionMat,
        vec3(1.5, -1.0, -1.5), vec3(0.7),
        glm::angleAxis(glm::radians(0.0f), vec3(0.0f, 1.0f, 0.0f)));

    // ── Spheres ───────────────────────────────────────────────────────────────
    // Glass sphere — center stage, slightly closer
    scene.addPrimitive(std::make_shared<Sphere>(dvec3(0.7, -0.35, -2.9), 0.65, glass));
    scene.addPrimitive(std::make_shared<Sphere>(dvec3(-0.45, -0.10, -4.4), 0.9, mirror));
    scene.addPrimitive(std::make_shared<Sphere>(dvec3(1.8, -0.35, -4.8), 0.65, green));
    // ── Floor ─────────────────────────────────────────────────────────────────
    scene.addPrimitive(std::make_shared<Triangle>(dvec3(-8.0, -1.0, 4.0), dvec3(8.0, -1.0, 4.0),
                                                  dvec3(8.0, -1.0, -12.0), gray));
    scene.addPrimitive(std::make_shared<Triangle>(dvec3(-8.0, -1.0, 4.0), dvec3(8.0, -1.0, -12.0),
                                                  dvec3(-8.0, -1.0, -12.0), gray));

    // ── Lighting ──────────────────────────────────────────────────────────────
    // Key light — right, warm
    scene.addLight(std::make_shared<RectLight>(
        dvec3(2.5, 5.0, 0.0), // origin (top-left corner of rect)
        dvec3(3.0, 0.0, 0.0), // edge1 → rightward
        dvec3(0.0, -2.0, 0.5), // edge2 → downward, slight fore-tilt
        Color(1.0, 0.95, 0.88), // warm white
        12.0)); // intensity (area-distributed, so lower than point)

    // Fill light — left, cool, narrower panel
    scene.addLight(std::make_shared<RectLight>(
        dvec3(-5.5, 3.0, -0.5), // origin
        dvec3(0.0, 2.0, 0.0), // edge1 → upward
        dvec3(0.0, 0.0, -2.0), // edge2 → into scene
        Color(0.88, 0.92, 1.0), // cool blue-white
        7.0));

    // Rim light — behind scene, small neutral strip along the back wall
    scene.addLight(std::make_shared<RectLight>(
        dvec3(-2.0, 4.5, -6.5), // origin
        dvec3(4.0, 0.0, 0.0), // edge1 → spans the scene width
        dvec3(0.0, -1.5, 0.0), // edge2 → short vertical drop
        Color(1.0, 1.0, 1.0), // neutral
        6.0));
    // Fill light — left, cooler
    scene.addLight(std::make_shared<PointLight>(
        dvec3(-4.0, 4.0, 1.0),
        Color(0.88, 0.92, 1.0), 20.0));

    // Rim/back light — behind scene, neutral
    scene.addLight(std::make_shared<PointLight>(
        dvec3(0.0, 6.0, -6.0),
        Color(1.0, 1.0, 1.0), 18.0));


    scene.addLight(std::make_shared<PointLight>(
        vec3(-2, -0, -2),
        Color(255.0 / 255.0, 70.0 / 255.0, 162.0 / 255.0), 5.0));

    scene.addLight(std::make_shared<PointLight>(
        vec3(-0.3, -0, -2),
        Color(255.0 / 255.0, 70.0 / 255.0, 162.0 / 255.0), 5.0));

    scene.addLight(std::make_shared<PointLight>(
        vec3(-2, 1, -2),

        Color(127.0 / 255.0, 0.0 / 255.0, 162.0 / 255.0), 5.0));

    scene.addLight(std::make_shared<PointLight>(
        vec3(-0.3, 1, -2),
        Color(127.0 / 255.0, 0.0 / 255.0, 255.0 / 255.0), 5.0));


    // ── Skybox & BVH ─────────────────────────────────────────────────────────
    scene.loadSkyBox(std::make_shared<Skybox>("../../assets/skybox"));
    scene.buildBVH();

    // ── Camera ────────────────────────────────────────────────────────────────
    const Camera camera(
        dvec3(-0.3, 1.1, 2.3), // eye — slightly higher for better angle
        dvec3(0.0, 0.2, -2.5), // target — looks into the scene
        dvec3(0.0, 1.0, 0.0), // up
        42.0, // slightly narrower FOV to tighten framing
        width,
        height,
        0.05, glm::distance(dvec3(-0.3, 1.1, 2.3), dvec3(-0.2, 0.2, -1.5))

    );

    return {std::move(scene), camera};
}

SceneSetup SceneFactory::createMaterialGridScene(const int width, const int height)
{
    Scene scene;

    // ── Floor ─────────────────────────────────────────────────────────────────
    auto tiled_tex = std::make_shared<ImageTexture>("../../assets/textures/painted_concrete_diff_1k.png");
    auto gray = std::make_shared<LambertMaterial>(tiled_tex);

    scene.addPrimitive(std::make_shared<Triangle>(dvec3(-10.0, -1.0, 6.0), dvec3(10.0, -1.0, 6.0),
                                                  dvec3(10.0, -1.0, -14.0), gray));
    scene.addPrimitive(std::make_shared<Triangle>(dvec3(-10.0, -1.0, 6.0), dvec3(10.0, -1.0, -14.0),
                                                  dvec3(-10.0, -1.0, -14.0), gray));

    // ── Material grid ─────────────────────────────────────────────────────────
    // Columns: roughness sweeps 0 (mirror-smooth) → 1 (fully rough)
    // Rows:    metalness sweeps 0 (dielectric)     → 1 (full metal)
    const int cols = 7;
    const int rows = 4;
    const float sphereR = 0.55;
    const float spacingX = 1.5;
    const float spacingZ = -1.8;
    const float startX = -spacingX * (cols - 1) * 0.5;
    const float startZ = -2.5;

    // Keep a consistent base color so only roughness/metalness vary visually
    auto baseAlbedo = std::make_shared<ConstantTexture>(Color(0.72, 0.45, 0.20)); // copper-ish hue
    auto flatNormal = std::make_shared<ConstantTexture>(Color(0.5, 0.5, 1.0));

    for (int row = 0; row < rows; ++row)
    {
        float metalness = rows > 1 ? powf(static_cast<float>(row) / (rows - 1), 2.2) : 0.0f;

        for (int col = 0; col < cols; ++col)
        {
            float roughness = cols > 1 ? static_cast<float>(col) / (cols - 1) : 0.0f;
            // Clamp away from exactly 0 to avoid degenerate GGX lobes
            roughness = std::max(roughness, 0.02f);

            GGXMaterial::Params p;
            Color albedoColor = (metalness > 0.5f)
                                    ? Color(0.95, 0.64, 0.10) // gold-ish for metal rows
                                    : Color(0.75, 0.10, 0.10); // red dielectric rows

            auto rowAlbedo = std::make_shared<ConstantTexture>(albedoColor);
            p.albedo = rowAlbedo;
            //p.normal    = flatNormal;
            p.roughness = std::make_shared<ConstantTexture>(Color(roughness, roughness, roughness));
            p.metallic = std::make_shared<ConstantTexture>(Color(metalness, metalness, metalness));

            auto mat = std::make_shared<GGXMaterial>(p);

            dvec3 pos(
                startX + col * spacingX,
                -1.0 + sphereR,
                startZ + row * spacingZ
            );

            scene.addPrimitive(std::make_shared<Sphere>(pos, sphereR, mat));
        }
    }

    // ── Lighting ──────────────────────────────────────────────────────────────
    // Broad key light, high and slightly forward, so roughness highlights are legible
    // scene.addLight(std::make_shared<RectLight>(
    //     dvec3(-3.0, 6.0, 2.0),
    //     dvec3(6.0, 0.0, 0.0),
    //     dvec3(0.0, -1.0, -1.5),
    //     Color(1.0, 0.97, 0.92),
    //     14.0));

    // Cool fill from the left to separate specular lobes from shadow
    scene.addLight(std::make_shared<PointLight>(
        dvec3(-6.0, 3.0, 1.0),
        Color(0.85, 0.9, 1.0), 18.0));

    // Soft rim from behind to pop the silhouettes against the back wall
    scene.addLight(std::make_shared<PointLight>(
        dvec3(0.0, 5.0, -8.0),
        Color(1.0, 1.0, 1.0), 16.0));

    // ── Skybox & BVH ─────────────────────────────────────────────────────────
    scene.loadSkyBox(std::make_shared<Skybox>("../../assets/skybox"));
    scene.buildBVH();
    // just top 3 levels, for a coarse look

    // ── Camera ────────────────────────────────────────────────────────────────
    const Camera camera(
        dvec3(0.0, 2.2, 6.0),
        dvec3(0.0, 0.0, -3.0),
        dvec3(0.0, 1.0, 0.0),
        38.0,
        width,
        height,
        0.24,
        glm::length(dvec3(0.0, 0.0, -3.0) - dvec3(0.0, 2.2, 6.0))
    );

    return {std::move(scene), camera};
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

    auto addBox = [&](const glm::dvec3& position,
                      const glm::dvec3& halfExtents,
                      double rotationYDegrees,
                      const std::shared_ptr<Material>& material)
    {
        scene.addPrimitive(
            std::make_shared<Box>(
                position,
                glm::angleAxis(
                    glm::radians(rotationYDegrees),
                    dvec3(0.0, 1.0, 0.0)),
                halfExtents,
                material));
    };

    // --------------------------------------------------
    // Dielectric materials
    // --------------------------------------------------
    auto greenGlass = createGlass(
        1.8,
        Color(0.2, 0.9, 0.3),
        0.35);

    auto amberGlass = createGlass(
        1.5,
        Color(0.9, 0.6, 0.1),
        0.2);

    auto deepBlue = createGlass(
        1.65,
        Color(0.1, 0.3, 0.9),
        0.3);

    auto rubyGlass = createGlass(
        1.779,
        Color(0.9, 0.05, 0.05),
        0.55);

    auto uraniumGlass = createGlass(
        1.5, // ior — ordinary glass range
        Color(0.85, 0.95, 0.3), // pale yellow-green tint
        0.35); // moderate density/absorption
    // --------------------------------------------------
    // Floor (rectangular)
    // --------------------------------------------------
    auto gray = std::make_shared<LambertMaterial>(
        Color(0.6, 0.6, 0.6));

    constexpr double kFloorY = -1.0;
    const dvec3 floorA(-8.0, kFloorY, -1.0);
    const dvec3 floorB(8.0, kFloorY, -1.0);
    const dvec3 floorC(8.0, kFloorY, -13.0);
    const dvec3 floorD(-8.0, kFloorY, -13.0);

    scene.addPrimitive(std::make_shared<Triangle>(
        floorA, floorB, floorC, gray));

    scene.addPrimitive(std::make_shared<Triangle>(
        floorA, floorC, floorD, gray));

    // --------------------------------------------------
    // Front row: dielectric spheres
    // --------------------------------------------------
    addSphere(
        dvec3(-1.0, -0.2, -4.0),
        0.8,
        amberGlass);

    addSphere(dvec3(1.0, -0.2, -4.0),
              0.8,
              deepBlue);

    addSphere(
        dvec3(3.0, -0.2, -4.0),
        0.8,
        rubyGlass);

    // --------------------------------------------------
    // Models
    // --------------------------------------------------
    loadModel(
        "../../assets/models/glass/glass.obj",
        deepBlue,
        dvec3(-3.0, -0.8, -4.0),
        vec3(0.7),
        25.0f);

    // --------------------------------------------------
    // Boxes (replacing the previous cube meshes)
    // --------------------------------------------------
    constexpr double kBoxHalfExtent = 0.4;
    const dvec3 boxHalfExtents(kBoxHalfExtent);
    const double boxCenterY = kFloorY + kBoxHalfExtent; // rest on the floor

    addBox(
        dvec3(-2.0, boxCenterY, -2.5),
        boxHalfExtents,
        -15.0,
        uraniumGlass);

    addBox(
        dvec3(2.0, boxCenterY, -2.5),
        boxHalfExtents,
        40.0,
        uraniumGlass);

    // --------------------------------------------------
    // Reference spheres (small background accents)
    // --------------------------------------------------
    addSphere(
        dvec3(-2.5, -0.4, -6.5),
        0.5,
        rubyGlass);

    addSphere(
        dvec3(0.0, -0.6, -6.5),
        0.3,
        greenGlass);

    addSphere(
        dvec3(2.5, -0.7, -6.5),
        0.2,
        greenGlass);

    // --------------------------------------------------
    // Lights
    // --------------------------------------------------
    // --------------------------------------------------
    // Lights
    // --------------------------------------------------

    // Key light (bright, slightly warm, camera-right)
    scene.addLight(std::make_shared<PointLight>(
        dvec3(4.0, 5.0, 0.0),
        Color(1.0, 1.0, 1.0),
        30.0));

    // Fill light (cooler, camera-left, softer)
    scene.addLight(std::make_shared<PointLight>(
        dvec3(-4.0, 5.0, 0.0),
        Color(0.8, 0.9, 1.0),
        20.0));

    // Front fill (low, warm, close to camera)
    scene.addLight(std::make_shared<PointLight>(
        dvec3(0.0, 3.0, 1.0),
        Color(1.0, 0.95, 0.8),
        10.0));

    // Rim/back light behind the boxes — catches glass edges and
    // separates the dark ruby/blue glass from the background
    scene.addLight(std::make_shared<PointLight>(
        dvec3(0.0, 3.5, -9.5),
        Color(0.9, 0.95, 1.0),
        18.0));

    // Overhead light above the front sphere row, for cleaner
    // caustics/highlights through the amber/blue/ruby spheres
    scene.addLight(std::make_shared<PointLight>(
        dvec3(0.5, 6.0, -4.0),
        Color(1.0, 1.0, 0.95),
        25.0));

    // Low warm accent near the floor, camera-right — adds a second
    // set of shadows and warms up the gray floor material
    scene.addLight(std::make_shared<PointLight>(
        dvec3(5.0, 1.2, -5.0),
        Color(1.0, 0.7, 0.4),
        6.0));

    // Small cool accent tucked behind the glass model, camera-left
    scene.addLight(std::make_shared<PointLight>(
        dvec3(-5.0, 1.5, -6.0),
        Color(0.5, 0.7, 1.0),
        6.0));
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
        dvec3(0.0, 2, 2.0),
        dvec3(0.0, 0.5, -4.5),
        dvec3(0.0, 1.0, 0.0),
        50.0,
        width,
        height
    );

    return {std::move(scene), camera};
}

SceneSetup SceneFactory::createAircraftScene(const int width, const int height)
{
    Scene scene;

    // --------------------------------------------------
    // Aircraft transform (shared by every part of the F-16 assembly:
    // fuselage, canopy, AIM-120, and jsm57 all sit in the same local frame)
    // --------------------------------------------------
    const dvec3 kAircraftPosition(-0.3, 0.0, -3.0);
    const vec3 kAircraftScale(0.03);
    const vec3 kAircraftRotationAxis = normalize(vec3(-0.1f, 1.0f, 0.f));
    const float kAircraftRotationAngle = -74.0f;
    const quat kAircraftRotation = glm::angleAxis(
        glm::radians(kAircraftRotationAngle),
        kAircraftRotationAxis);

    // --------------------------------------------------
    // Helpers
    // --------------------------------------------------
    auto createGlass = [](
        double ior,
        const Color& tint,
        double density)
    {
        return std::make_shared<DielectricMaterial>(ior, tint, density);
    };

    auto addSphere = [&](const dvec3& position,
                         double radius,
                         const std::shared_ptr<Material>& material)
    {
        scene.addPrimitive(
            std::make_shared<Sphere>(
                position,
                radius,
                material));
    };

    auto white = std::make_shared<LambertMaterial>(Color(1));

    auto loadModel = [&](const char* path,
                         const std::shared_ptr<Material>& material,
                         const vec3& position,
                         const vec3& scale,
                         const vec3& axis,
                         const float rotation)
    {
        ObjLoader::load(
            path,
            scene,
            material,
            position,
            scale,
            glm::angleAxis(glm::radians(rotation), axis));
    };

    auto makeTex = [](const char* path)
    {
        return std::make_shared<ImageTexture>(path);
    };

    // Places a point light at localPos (in the aircraft's local model space)
    // and carries it through the same scale/rotate/translate transform as
    // the mesh, so it stays locked to the airframe.
    auto addAircraftLight = [&](const vec3& localPos,
                                const Color& color,
                                double intensity)
    {
        vec3 worldPos = vec3(kAircraftPosition) + kAircraftRotation * (kAircraftScale * localPos);
        scene.addLight(std::make_shared<PointLight>(dvec3(worldPos), color, intensity));

        // addSphere(worldPos, 0.1, white);
    };

    // --------------------------------------------------
    // Dielectric materials
    // --------------------------------------------------
    auto greenGlass = createGlass(1.5, Color(1), 0.15);

    // --------------------------------------------------
    // GGX (albedo/roughness/metallic/normal) materials
    // --------------------------------------------------
    GGXMaterial::Params planeParams;
    GGXMaterial::Params aim120Params;
    GGXMaterial::Params jsm57Params;
    try
    {
        planeParams.albedo = makeTex("../../assets/models/F16/F16_albedo.png");
        planeParams.roughness = makeTex("../../assets/models/F16/F16_roughness.png");
        planeParams.metallic = makeTex("../../assets/models/F16/F16_metalness.png");
        planeParams.normal = makeTex("../../assets/models/F16/F16_normal.png");

        aim120Params.albedo = makeTex("../../assets/models/aim120/AIM120_albedo.png");
        aim120Params.roughness = makeTex("../../assets/models/aim120/AIM120_roughness.png");
        aim120Params.metallic = makeTex("../../assets/models/aim120/AIM120_metallic.png");
        aim120Params.normal = makeTex("../../assets/models/aim120/AIM120_normal.png");

        jsm57Params.albedo = makeTex("../../assets/models/jsm57/jsm57_Albedo.png");
        jsm57Params.roughness = makeTex("../../assets/models/jsm57/jsm57_Roughness.png");
        jsm57Params.metallic = makeTex("../../assets/models/jsm57/jsm57_Metallic.png");
        jsm57Params.normal = makeTex("../../assets/models/jsm57/jsm57_Normal.png");
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << '\n';
    }

    auto planeMat = std::make_shared<GGXMaterial>(planeParams);
    auto aim120Mat = std::make_shared<GGXMaterial>(aim120Params);
    auto jsm57Mat = std::make_shared<GGXMaterial>(jsm57Params);

    // --------------------------------------------------
    // Floor
    // --------------------------------------------------

    // --------------------------------------------------
    // Models — all share the same position/scale/rotation
    // --------------------------------------------------
    loadModel(
        "../../assets/models/F16/canopylp.obj",
        greenGlass,
        kAircraftPosition, kAircraftScale, kAircraftRotationAxis, kAircraftRotationAngle);

    loadModel(
        "../../assets/models/F16/F-16.obj",
        planeMat,
        kAircraftPosition, kAircraftScale, kAircraftRotationAxis, kAircraftRotationAngle);

    loadModel(
        "../../assets/models/aim120/aim120.obj",
        aim120Mat,
        kAircraftPosition, kAircraftScale, kAircraftRotationAxis, kAircraftRotationAngle);

    loadModel(
        "../../assets/models/jsm57/jsm57.obj",
        jsm57Mat,
        kAircraftPosition, kAircraftScale, kAircraftRotationAxis, kAircraftRotationAngle);

    // --------------------------------------------------
    // Scene lights (studio rig)
    // --------------------------------------------------

    // Key light — warm, top-right
    scene.addLight(std::make_shared<RectLight>(
        dvec3(2.5, 5.0, 0.0), // origin (top-left corner of rect)
        dvec3(3.0, 0.0, 0.0), // edge1 → rightward
        dvec3(0.0, -2.0, 0.5), // edge2 → downward, slight fore-tilt
        Color(1.0, 0.95, 0.88), // warm white
        12.0)); // intensity (area-distributed, so lower than point)

    // Fill light — left, cool, narrower panel
    scene.addLight(std::make_shared<RectLight>(
        dvec3(-5.5, 3.0, -0.5), // origin
        dvec3(0.0, 2.0, 0.0), // edge1 → upward
        dvec3(0.0, 0.0, -2.0), // edge2 → into scene
        Color(0.88, 0.92, 1.0), // cool blue-white
        7.0));

    // Rim light — behind scene, small neutral strip along the back wall
    scene.addLight(std::make_shared<RectLight>(
        dvec3(-2.0, 4.5, -6.5), // origin
        dvec3(4.0, 0.0, 0.0), // edge1 → spans the scene width
        dvec3(0.0, -1.5, 0.0), // edge2 → short vertical drop
        Color(1.0, 1.0, 1.0), // neutral
        6.0));

    // Fill point light — left, cooler
    scene.addLight(std::make_shared<PointLight>(
        dvec3(-4.0, 4.0, 1.0),
        Color(0.88, 0.92, 1.0), 0.0));

    // Rim/back point light — behind scene, neutral
    scene.addLight(std::make_shared<PointLight>(
        dvec3(0.0, 6.0, -6.0),
        Color(1.0, 1.0, 1.0), 18.0));

    // --------------------------------------------------
    // Aircraft nav / anti-collision lights (locked to airframe transform)
    //
    // Measured dimensions (length 3.52 / width 2.391 / height 1.034) are
    // raw OBJ (local model-space) units — i.e. already in the same space
    // as the localPos argument to addAircraftLight, before kAircraftScale
    // is applied. So we just halve them directly, no division by scale.
    // --------------------------------------------------
    const float kHalfWidth = 2.351f / 2.0f; // = 1.1955 — wingtip to wingtip
    const float kHalfHeight = 1.3f / 2.0f; // = 0.517  — belly to spine
    const float kHalfLength = 3.520f / 2.0f; // = 1.76   — nose to tail

    addAircraftLight(vec3(-2 * kHalfLength, -3 * kHalfHeight, 37 * kHalfWidth), Color(1.0, 0.0, 0.0), 0.4);
    // belly beacon
    addAircraftLight(vec3(-2 * kHalfLength, -3 * kHalfHeight, -37 * kHalfWidth), Color(0.0, 0.0, 1.0), 0.4);
    // belly beacon


    addAircraftLight(vec3(26 * kHalfLength, -3.2 * kHalfHeight, 7 * kHalfWidth), Color(1.0, 0.0, 0.0), 0.02);
    // belly beacon
    addAircraftLight(vec3(26 * kHalfLength, -3.2 * kHalfHeight, -7 * kHalfWidth), Color(0.0, 0.0, 1.0), 0.02);
    // belly beacon


    addAircraftLight(vec3(36 * kHalfLength, 3.6 * kHalfHeight, 0), Color(0.0, 1.0, 0.0), 0.6); // belly beacon
    addAircraftLight(vec3(-26 * kHalfLength, 48 * kHalfHeight, 1.2), Color(1.0, 1.0, 1.0), 0.5); // belly beacon

    // addAircraftLight(vec3( -0.702, 0.0, -1.143),        Color(0.0, 0.0, 1.0), 3.0); // belly beacon
    // addAircraftLight(vec3( -0.702, 0.0, 1.143),        Color(1.0, 0.0, 0.0), 3.0); // belly beacon
    // --------------------------------------------------
    // Environment
    // --------------------------------------------------
    auto skybox = std::make_shared<Skybox>("../../assets/skybox2");
    scene.loadSkyBox(skybox);
    scene.buildBVH();

    // --------------------------------------------------
    // Camera
    // --------------------------------------------------
    const Camera camera(
        dvec3(-1.2, 0.65, 1.4),
        dvec3(0.7, 0.6, -4.5),
        dvec3(0.0, 1.0, 0.0),
        48.0,
        width,
        height,
        0.008,
        glm::distance(
            dvec3(-1.2, 0.65, 1.4),

            dvec3(0.7, 0.6, -4.5)));

    return {std::move(scene), camera};
}

SceneSetup SceneFactory::createStormShadowScene(const int width, const int height)
{
    Scene scene;

    // ── Material ──────────────────────────────────────────────────────────────
    auto makeTex = [](const char* path)
    {
        return std::make_shared<ImageTexture>(path);
    };

    GGXMaterial::Params stormParams;
    try
    {
        stormParams.albedo = makeTex("../../assets/models/storm-shadow/StormShadow_Albedo.png");
        stormParams.roughness = makeTex("../../assets/models/storm-shadow/StormShadow_Roughness.png");
        stormParams.metallic = makeTex("../../assets/models/storm-shadow/StormShadow_Metallic.png");
        stormParams.normal = makeTex("../../assets/models/storm-shadow/StormShadow_Normal.png");
        stormParams.ambient_occlusion = makeTex("../../assets/models/storm-shadow/StormShadow_AmbientOcclusion.png");
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
    auto stormMat = std::make_shared<GGXMaterial>(stormParams);

    ObjLoader::load(
        "../../assets/models/storm-shadow/StormShadow_simplified.obj", scene, stormMat,
        vec3(0.0, 0, -3.0), // position
        vec3(1.2), // scale — adjust once you see the model size
        glm::angleAxis(glm::radians(-50.0f), vec3(0.0f, 1.0f, 0.0f)));

    // ── Floor ─────────────────────────────────────────────────────────────────
    auto gray = std::make_shared<LambertMaterial>(Color(0.5, 0.5, 0.5));
    scene.addPrimitive(std::make_shared<Triangle>(dvec3(-8.0, -1.0, 4.0), dvec3(8.0, -1.0, 4.0),
                                                  dvec3(8.0, -1.0, -12.0), gray));
    scene.addPrimitive(std::make_shared<Triangle>(dvec3(-8.0, -1.0, 4.0), dvec3(8.0, -1.0, -12.0),
                                                  dvec3(-8.0, -1.0, -12.0), gray));

    // ── Lighting ──────────────────────────────────────────────────────────────
    scene.addLight(std::make_shared<RectLight>(
        dvec3(2.5, 2.0, 1.0),
        dvec3(3.0, 0.0, 0.0),
        dvec3(0.0, -2.0, 0.5),
        Color(1.0, 0.95, 0.88),
        12.0));

    scene.addLight(std::make_shared<RectLight>(
        dvec3(-5.5, 3.0, -0.5),
        dvec3(0.0, 2.0, 0.0),
        dvec3(0.0, 0.0, -2.0),
        Color(0.88, 0.92, 1.0),
        7.0));

    // ── Skybox & BVH ──────────────────────────────────────────────────────────
    scene.loadSkyBox(std::make_shared<Skybox>("../../assets/skybox"));
    scene.buildBVH();

    // ── Camera ────────────────────────────────────────────────────────────────
    const Camera camera(
        dvec3(-2.3, 2.5, 4.3),
        dvec3(0.6, 0.0, -3.5),
        dvec3(0.0, 1.0, 0.0),
        42.0,
        width,
        height
    );

    return {std::move(scene), camera};
}
