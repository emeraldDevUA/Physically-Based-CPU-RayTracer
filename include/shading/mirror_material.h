//
// Created by Asus on 5/24/2026.
//


#pragma once
#include "material.h"

class MirrorMaterial : public Material
{
public:
    MaterialType type() const override {
        return MaterialType::Mirror;
    }
    explicit MirrorMaterial(const vec3 reflectance = vec3(1.0))
        : m_reflectance(reflectance) {}

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
    vec3 m_reflectance;

};


inline vec3 reflect(vec3 v, vec3 n) {
    return v -  2.0f * n * dot(v, n) ;
}