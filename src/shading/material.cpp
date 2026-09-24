// Material implementation for the Global Illumination starter.

#include "shading/material.h"

#include <glm/glm.hpp>
#include <core/random.h>
#include "core/constants.h"
#include "shading/auxhiliary_methods.h"


MaterialSample Material::sample(const HitRecord& rec,
                                const dvec3& wo) const
{
    // Cosine-weighted hemisphere sample in tangent space
    const double r1 = randomDouble();
    const double r2 = randomDouble();

    const double sinTheta = std::sqrt(1.0 - r2);
    const double cosTheta = std::sqrt(r2); // cos²θ = r2
    const double phi = 2.0 * constants::kPi * r1;

    const dvec3 localWi = {
        sinTheta * std::cos(phi),
        sinTheta * std::sin(phi),
        cosTheta
    };

    // Transform to world space via ONB around the shading normal
    dvec3 T, B;
    const dvec3 N = normalize(rec.geometricNormal);
    buildTBN(N, T, B);

    const dvec3 wi = normalize(
        T * localWi.x + B * localWi.y + N * localWi.z
    );

    MaterialSample ms;
    ms.wi = wi;
    ms.weight = evaluate(rec, wo, wi) / pdf(rec, wo, wi);
    ms.delta = false;
    return ms;
}

double Material::pdf(const HitRecord& rec,
                     const dvec3& wo,
                     const dvec3& wi) const
{
    const double cosTheta = glm::dot(normalize(rec.geometricNormal),
                                     normalize(wi));


    // cosine-weighted hemisphere sampling, it is used with lambert materials.
    // essentially it is just max(N dot L, 0)
    return std::max(cosTheta, 0.0) / constants::kPi;
}

LambertMaterial::LambertMaterial(const Color& color)
    : m_texture(std::make_shared<ConstantTexture>(color))
{
}

LambertMaterial::LambertMaterial(std::shared_ptr<Texture> texture)
    : m_texture(std::move(texture))
{
}

Color LambertMaterial::albedo(const HitRecord& rec) const
{
    return m_texture->value(rec.uv, rec.position);
}


Color LambertMaterial::evaluate(const HitRecord& rec,
                                const dvec3&,
                                const dvec3& wi) const
{
    const double cosTheta = glm::dot(rec.shadingNormal, wi);

    if (cosTheta <= 0.0)
    {
        return {0.0, 0.0, 0.0};
    }

    return albedo(rec) / constants::kPi;
}
