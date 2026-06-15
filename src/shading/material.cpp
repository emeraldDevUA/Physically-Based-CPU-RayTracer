// Material implementation for the Global Illumination starter.

#include "shading/material.h"

#include <glm/glm.hpp>
#include <core/random.h>
#include "core/constants.h"

static void buildTBN(const glm::dvec3& N,
                     glm::dvec3& T,
                     glm::dvec3& B)
{
    // Frisvad's method — numerically stable except near (0, 0, -1)
    if (N.z < -0.9999999) {
        T = glm::dvec3( 0.0, -1.0, 0.0);
        B = glm::dvec3(-1.0,  0.0, 0.0);
        return;
    }

    const double a = 1.0 / (1.0 + N.z);
    const double b = -N.x * N.y * a;

    T = glm::dvec3(1.0 - N.x * N.x * a,  b,              -N.x);
    B = glm::dvec3(b,                      1.0 - N.y*N.y*a, -N.y);
}



MaterialSample Material::sample(const HitRecord& rec,
                                const glm::dvec3& wo) const {
    // Cosine-weighted hemisphere sample in tangent space
    const double r1  = randomDouble();
    const double r2  = randomDouble();

    const double sinTheta = std::sqrt(1.0 - r2);
    const double cosTheta = std::sqrt(r2);          // cos²θ = r2
    const double phi      = 2.0 * constants::kPi * r1;

    const glm::dvec3 localWi = {
        sinTheta * std::cos(phi),
        sinTheta * std::sin(phi),
        cosTheta
    };

    // Transform to world space via ONB around the shading normal
    glm::dvec3 T, B;
    const glm::dvec3 N = glm::normalize(rec.geometricNormal);
    buildTBN(N, T, B);

    const glm::dvec3 wi = glm::normalize(
        T * localWi.x + B * localWi.y + N * localWi.z
    );

    MaterialSample ms;
    ms.wi     = wi;
    ms.weight = evaluate(rec, wo, wi) / pdf(rec, wo, wi);
    ms.delta  = false;
    return ms;
}

double Material::pdf(const HitRecord& rec,
                     const glm::dvec3& wo,
                     const glm::dvec3& wi) const {
    const double cosTheta = glm::dot(glm::normalize(rec.geometricNormal),
                                     glm::normalize(wi));


    // cosine-weighted hemisphere sampling, it is used with lambert materials.
    // essentially it is just max(N dot L, 0)
    return std::max(cosTheta, 0.0) / constants::kPi;
}

LambertMaterial::LambertMaterial(const Color& color)
    : m_texture(std::make_shared<ConstantTexture>(color)) {}

LambertMaterial::LambertMaterial(std::shared_ptr<Texture> texture)
    : m_texture(std::move(texture)) {}

Color LambertMaterial::albedo(const HitRecord& rec) const {
    return m_texture->value(rec.uv, rec.position);
}


Color LambertMaterial::evaluate(const HitRecord& rec,
                                const glm::dvec3&,
                                const glm::dvec3& wi) const {

    const double cosTheta = glm::dot(rec.shadingNormal, wi);
    
    if (cosTheta <= 0.0) {
        return {0.0, 0.0, 0.0};
    }

    return albedo(rec) / constants::kPi;
}
