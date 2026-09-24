// Direct illumination integrator with hard shadows.

#include "render/direct_lighting_integrator.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <memory>

#include "scene/scene.h"
#include "shading/light.h"
#include "shading/material.h"
#include "scene/hit_record.h"
#include "core/constants.h"

DirectLightingIntegrator::DirectLightingIntegrator(const int maxDepth,
                                                   const Color& background)
    : m_maxDepth(maxDepth), m_background(background) {}

Color DirectLightingIntegrator::Li(const Ray& ray,
                                   const Scene& scene,
                                   const int depth) const {
    if (depth > m_maxDepth) {
        return Color{0.0, 0.0, 0.0};
    }

    HitRecord rec;

    if (!scene.intersect(ray, rec)) {
        return m_background;
    }

    Color result = rec.material ? rec.material->emission(rec) : Color(0.0, 0.0, 0.0);

    dvec3 wo = -ray.direction;

    for (const auto& lightBase : scene.lights()) {
        const auto pointLight = std::dynamic_pointer_cast<PointLight>(lightBase);
        if (!pointLight) {
            continue;
        }

        const dvec3 lightVec = pointLight->position() - rec.position; // note: reversed from yours
        const double distToLight  = glm::length(lightVec);
        const dvec3 wi       = lightVec / distToLight; // unit direction toward light

        // Shadow ray — offset origin slightly to avoid self-intersection
        const Ray shadowRay{
            rec.position + rec.geometricNormal * constants::kEpsilon,
            wi,
            constants::kEpsilon,
            distToLight - constants::kEpsilon  // don't hit the light itself
        };

        HitRecord shadowRec;
        if (scene.intersect(shadowRay, shadowRec)) {
            continue; // occluded — skip this light
        }

        // Lambertian cosine term
        const double cosTheta = std::max(0.0, glm::dot(rec.geometricNormal, wi));

        // Inverse-square attenuation
        const double attenuation = 1.0 / (distToLight * distToLight);

        // Evaluate material BRDF and accumulate
        if (rec.material) {
            const Color brdf = rec.material->evaluate(rec, wo, wi);
            result += brdf * pointLight->intensity() * cosTheta * attenuation;
        }
    }

    return result;
}
