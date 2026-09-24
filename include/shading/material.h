// Material describes how a surface scatters and emits light.

#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "core/color.h"
#include "scene/hit_record.h"
#include "shading/texture.h"


enum class MaterialType
{
    Lambert,
    Mirror,
    Dielectric,
    Emissive,
    GGX, // rough/metal unified
    BlinnPhong,
    Unknown
};


using glm::vec3;
using glm::dvec3;

using glm::clamp;
using glm::normalize;

using std::shared_ptr;


struct MaterialSample
{
    dvec3 wi{0.0, 1.0, 0.0}; // Sampled incoming direction in world space.

    Color weight{0.0, 0.0, 0.0};
    // Throughput multiplier carried by this sample. Multiplies the output which dies if too smal;

    double pdf = 0.0; // Probability density for the sampled direction. Used to correct bias in montecarlo integration

    bool valid = false; // set as false if you want to indirect illumination after

    bool delta = false; // For dirac delta materials
};


class Material
{
public:
    virtual ~Material() = default;

    // Returns the high-level material category.
    virtual MaterialType type() const
    {
        return MaterialType::Unknown;
    }

    // Base surface color used by simple shading models.
    virtual Color albedo(const HitRecord& rec) const = 0;

    // Self-emission. Default is black.
    virtual Color emission(const HitRecord&) const
    {
        return {0.0, 0.0, 0.0};
    }

    virtual dvec3 normal(const HitRecord& rec) const
    {
        return rec.geometricNormal;
    }

    // Evaluate the scattering function f(wo, wi).
    virtual Color evaluate(const HitRecord& rec,
                           const dvec3& wo,
                           const dvec3& wi) const = 0;

    // Sample one incoming direction. This matters once stochastic integrators are added.
    virtual MaterialSample sample(const HitRecord& rec,
                                  const dvec3& wo) const;

    // PDF associated with sample().
    virtual double pdf(const HitRecord& rec,
                       const dvec3& wo,
                       const dvec3& wi) const;

    // Whether the material behaves as a delta distribution.
    virtual bool isDelta() const
    {
        return false;
    }
};

class LambertMaterial : public Material
{
public:
    explicit LambertMaterial(const Color& color);
    explicit LambertMaterial(std::shared_ptr<Texture> texture);

    MaterialType type() const override
    {
        return MaterialType::Lambert;
    }

    Color albedo(const HitRecord& rec) const override;
    Color evaluate(const HitRecord& rec,
                   const dvec3& wo,
                   const dvec3& wi) const override;

private:
    std::shared_ptr<Texture> m_texture;
};
