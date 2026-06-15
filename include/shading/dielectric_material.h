

#pragma once
#include "material.h"

class DielectricMaterial : public Material
{
public:
    MaterialType type() const override {
        return MaterialType::Dielectric;
    }
    explicit DielectricMaterial(double ior,
                             Color absorptionColor = Color(1.0),  // white = no tint
                             double absorptionDensity = 0.0)      // 0 = no absorption
    : m_ior(ior)
    , m_absorptionColor(absorptionColor)
    , m_absorptionDensity(absorptionDensity)
    {}

    Color evaluate(const HitRecord& rec,
               const glm::dvec3& wo,
               const glm::dvec3& wi) const override;

    MaterialSample sample(const HitRecord& rec,
                          const glm::dvec3& wo) const override;

    double pdf(const HitRecord& rec,
               const glm::dvec3& wo,
               const glm::dvec3& wi) const override;

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
        if (eta > 1.0) {
            double sinT2 = eta * eta * (1.0 - cosine * cosine);
            if (sinT2 > 1.0) return 1.0; // TIR — full reflection (redundant but safe)
            cosine = std::sqrt(1.0 - sinT2);
        }

        return r0 + (1.0 - r0) * std::pow(1.0 - cosine, 5.0);
    }

    static glm::dvec3 refract(const glm::dvec3& uv,
                             const glm::dvec3& n,
                             double etaRatio)
    {
        double cosTheta = glm::dot(-uv, n);
        cosTheta = std::min(cosTheta, 1.0);

        glm::dvec3 rPerp = etaRatio * (uv + cosTheta * n);

        double k = 1.0 - glm::dot(rPerp, rPerp);
        if (k < 0.0)
            return glm::dvec3(0.0); // signal TIR failure

        glm::dvec3 rParallel = -std::sqrt(k) * n;

        return rPerp + rParallel;
    }
};
