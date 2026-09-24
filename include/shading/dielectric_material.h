

#pragma once
#include "material.h"

class DielectricMaterial : public Material
{
public:
    MaterialType type() const override {
        return MaterialType::Dielectric;
    }
    explicit DielectricMaterial(const double ior,
                             const Color& absorptionColor = Color(1.0),  // white = no tint
                             const double absorptionDensity = 0.0)      // 0 = no absorption
    : m_ior(ior)
    , m_absorptionColor(absorptionColor)
    , m_absorptionDensity(absorptionDensity)
    {}

    Color evaluate(const HitRecord& rec,
               const dvec3& wo,
               const dvec3& wi) const override;

    MaterialSample sample(const HitRecord& rec,
                          const dvec3& wo) const override;

    double pdf(const HitRecord& rec,
               const dvec3& wo,
               const dvec3& wi) const override;

    bool isDelta() const override;

    Color albedo(const HitRecord& rec) const override;

private:
    double m_ior;
    Color  m_absorptionColor;    // the color the glass appears (e.g. green for green glass)
    double m_absorptionDensity;  // how quickly it absorbs — higher = darker faster

    // Schlick Fresnel approximation.
    static double schlick(double cosine, double eta) {
        // r0 is symmetric, so (1-eta)/(1+eta) == (eta-1)/(eta+1) squared
        double r0 = (1.0 - eta) / (1.0 + eta);
        r0 = r0 * r0;

        // When exiting (eta > 1), use the transmitted cosine instead
            const double sinT2 = eta * eta * (1.0 - cosine * cosine);
            if (sinT2 > 1.0) return 1.0; // TIR — full reflection (redundant but safe)
            cosine = std::sqrt(1.0 - sinT2);


        return r0 + (1.0 - r0) * std::pow(1.0 - cosine, 5.0);
    }

    static dvec3 refract(const dvec3& uv,
                             const dvec3& n,
                             const double etaRatio)
    {
        double cosTheta = glm::dot(-uv, n);
        cosTheta = std::min(cosTheta, 1.0);

        const dvec3 rPerp = etaRatio * (uv + cosTheta * n);

        const double k = 1.0 - glm::dot(rPerp, rPerp);
        if (k < 0.0)
            return dvec3(0.0); // signal TIR failure

        const dvec3 rParallel = -std::sqrt(k) * n;

        return rPerp + rParallel;
    }
};
