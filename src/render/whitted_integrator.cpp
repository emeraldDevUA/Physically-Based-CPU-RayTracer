// Starter stub for a later Whitted-style integrator.
// Returning black keeps the executable buildable before the module is implemented.

#include "render/whitted_integrator.h"

#include "core/random.h"
#include "scene/scene.h"
#include "shading/ggx_material.h"
#include "shading/material.h"

WhittedIntegrator::WhittedIntegrator(int maxDepth, const Color& background)
    : m_maxDepth(maxDepth), m_background(background)
{
}

Color WhittedIntegrator::Li(const Ray& initialRay, const Scene& scene, int) const
{
    const auto skybox = scene.getSkyBox();

    Color radiance(0.0);
    Color throughput(1.0);

    Ray ray = initialRay;

    for (int depth = 0; depth <= m_maxDepth; depth++)
    {
        HitRecord rec;

        if (!scene.intersect(ray, rec))
        {
            radiance += throughput *
                (skybox ? skybox->sample(ray.direction) : m_background);
            break;
        }

        auto material = rec.material;

        if (!material)
            break;

        rec.geometricNormal = material->normal(rec);

        //-------------------------------------
        // Emission
        //-------------------------------------
        radiance += throughput * material->emission(rec);

        //-------------------------------------
        // Direct Lighting
        //-------------------------------------

        if (!material->isDelta())
        {
            for (const auto& light : scene.lights())
            {
                // --- Point light path (unchanged) ---
                if (const auto pointLight =
                    std::dynamic_pointer_cast<PointLight>(light))
                {
                    dvec3 lightVec = pointLight->position() - rec.position;
                    double distToLight = glm::length(lightVec);
                    double invDist = 1.0 / distToLight;
                    dvec3 wi = lightVec * invDist;

                    Ray shadowRay{
                        rec.position + rec.geometricNormal * constants::kEpsilon,
                        wi,
                        constants::kEpsilon,
                        distToLight - constants::kEpsilon
                    };

                    HitRecord shadowRec;
                    if (scene.intersect(shadowRay, shadowRec))
                        continue;

                    double cosTheta = std::max(0.0, glm::dot(rec.geometricNormal, wi));
                    double attenuation = invDist * invDist;
                    Color brdf = material->evaluate(rec, -ray.direction, wi);

                    radiance +=
                        throughput * brdf * pointLight->intensity() * cosTheta * attenuation;
                }
                // --- Area light path ---
                else if (const auto areaLight =
                    std::dynamic_pointer_cast<RectLight>(light))
                {
                    const int N = 4; // e.g. 4, 8, 16...
                    Color Lo(0.0);

                    for (int i = 0; i < N; i++)
                    {
                        const Light::Sample ls =
                            areaLight->samplePoint(randomFloat(), randomFloat());

                        dvec3 toLight = ls.position - rec.position;
                        double distSq = glm::dot(toLight, toLight);
                        double dist = std::sqrt(distSq);
                        dvec3 wi = toLight / dist;

                        double cosTheta = std::max(0.0, glm::dot(rec.geometricNormal, wi));
                        double cosLight = std::max(0.0, glm::dot(ls.normal, -wi));

                        if (cosTheta < 1e-9 || cosLight < 1e-9)
                            continue;

                        Ray shadowRay{
                            rec.position + rec.geometricNormal * constants::kEpsilon,
                            wi,
                            constants::kEpsilon,
                            dist - constants::kEpsilon
                        };

                        HitRecord shadowRec;
                        if (scene.intersect(shadowRay, shadowRec))
                            continue;

                        double G = (cosTheta * cosLight) / distSq;
                        Color brdf = material->evaluate(rec, -ray.direction, wi);

                        Lo += brdf * ls.emission * (G / ls.pdf);
                    }

                    radiance += throughput * (Lo / static_cast<double>(N));
                }
            }
        }

        //-------------------------------------
        // Sample next bounce
        //-------------------------------------

        // Diffuse materials shouldn't be raytraced, because they reflect light every direction, so it
        // would be very inefficient
        if (material->type() == MaterialType::Lambert)
            break;


        MaterialSample ms = material->sample(rec, -ray.direction);

        if (!ms.valid) break;

        throughput *= ms.weight;

        throughput.r = std::min(throughput.r, 10.0);
        throughput.g = std::min(throughput.g, 10.0);
        throughput.b = std::min(throughput.b, 10.0);

        dvec3 offsetNormal =
            glm::dot(ms.wi, rec.geometricNormal) > 0.0 ? rec.geometricNormal : -rec.geometricNormal;

        ray = Ray{
            rec.position +
            offsetNormal * constants::kEpsilon,
            ms.wi
        };


        if (depth > 1)
        {
            auto q = static_cast<float>(1.0f - std::max({throughput.r, throughput.g, throughput.b}));
            q = clamp(q, 0.05f, 0.95f);
            // never kill everything, never keep everything
            if (randomFloat() < q)
                break; // ray dies

            throughput /= (1.0f - q);
            // survivor is boosted to compensate
        }
    }

    return radiance;
}
