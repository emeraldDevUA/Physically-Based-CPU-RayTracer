//
// Created by Asus on 5/31/2026.
//


#pragma once
#include "material.h"


class EmissiveMaterial : public Material
{
public:
    MaterialType type() const override {
        return MaterialType::Emissive;
    }
    explicit EmissiveMaterial(const Color emission = Color(1.0))
        : m_color(emission) {}

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
    Color m_color;

};
