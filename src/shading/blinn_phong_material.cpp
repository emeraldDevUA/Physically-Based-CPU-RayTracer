//
// Created by Asus on 6/19/2026.
//


#include "shading/blinn_phong_material.h"

#include "core/constants.h"
#include "core/random.h"
#include "shading/auxhiliary_methods.h"


BlinnPhongMaterial::BlinnPhongMaterial(const Params& params)
{
    this->m_params = params;
}


Color BlinnPhongMaterial::diffuse(const HitRecord& rec) const
{
    return m_params.diffuse->value(rec.uv, rec.position);
}

Color BlinnPhongMaterial::specular(const HitRecord& rec) const
{
    return m_params.specular->value(rec.uv, rec.position);
}

Color BlinnPhongMaterial::ambient(const HitRecord& rec) const
{
    return m_params.ambient->value(rec.uv, rec.position);
}

Color BlinnPhongMaterial::evaluate(const HitRecord& rec,
                                   const dvec3& wo,
                                   const dvec3& wi) const
{
    Color albedo = rec.material->albedo(rec);
    dvec3 normal = rec.material->normal(rec);

    const Color diffuse_component = m_params.k_d * diffuse(rec);
    const Color specular_component = m_params.k_s * specular(rec);
    const Color ambient_component = m_params.k_a * ambient(rec);

    const dvec3 half_vector = normalize(wo + wi);

    const auto specular_to_pwr = specular_component *
        pow(glm::max(glm::dot(normal, half_vector), 0.0), m_params.shininess);

    const auto diffuse_cut_off = albedo * diffuse_component *
        glm::max(glm::dot(wi, normal), 0.0);

    return ambient_component + diffuse_cut_off + specular_to_pwr;
}

Color BlinnPhongMaterial::albedo(const HitRecord& rec) const
{
    if (!m_params.albedo)
        return Color(0.0);

    return this->m_params.albedo->value(rec.uv, rec.position);
}


MaterialSample BlinnPhongMaterial::sample(const HitRecord& rec,
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
    ms.valid = true;
    return ms;
}

double BlinnPhongMaterial::pdf(const HitRecord& rec,
                               const dvec3& wo,
                               const dvec3& wi) const
{
    const double cosTheta = glm::dot(normalize(rec.geometricNormal),
                                     normalize(wi));


    // cosine-weighted hemisphere sampling, it is used with lambert materials.
    // essentially it is just max(N dot L, 0)
    return std::max(cosTheta, 0.0) / constants::kPi;
}
