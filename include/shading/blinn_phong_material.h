//
// Created by Asus on 6/19/2026.
//


#pragma once
#include "material.h"


class BlinnPhongMaterial : public Material
{
public:
    struct Params
    {
        std::shared_ptr<Texture> albedo;
        std::shared_ptr<Texture> normal;

        std::shared_ptr<Texture> diffuse;
        std::shared_ptr<Texture> specular;
        std::shared_ptr<Texture> ambient;

        double k_d = 0.8, k_s = 0.2, k_a = 0.1;
        double shininess = 64;
    };

    MaterialType type() const override
    {
        return MaterialType::BlinnPhong;
    }


    Color evaluate(const HitRecord& rec,
                   const dvec3& wo,
                   const dvec3& wi) const override;

    MaterialSample sample(const HitRecord& rec,
                          const dvec3& wo) const override;

    double pdf(const HitRecord& rec,
               const dvec3& wo,
               const dvec3& wi) const override;


    Color diffuse(const HitRecord& rec) const;
    Color specular(const HitRecord& rec) const;
    Color ambient(const HitRecord& rec) const;
    Color albedo(const HitRecord& hit) const override;
    BlinnPhongMaterial(const Params& params);

private:
    Params m_params{};
};
