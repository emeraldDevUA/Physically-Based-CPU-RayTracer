//
// Created by Asus on 6/19/2026.
//
#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "material.h"
#include "core/color.h"
#include "scene/hit_record.h"
#include "shading/texture.h"



class GGXMaterial : public Material
{
public:
    struct Params
    {
        shared_ptr<Texture> albedo;
        shared_ptr<Texture> roughness; // R channel usually
        shared_ptr<Texture> metallic; // single channel
        shared_ptr<Texture> normal; // tangent-space normal map
        shared_ptr<Texture> emission; // for emissive surfaces

        // optional extras:
        shared_ptr<Texture> ambient_occlusion;       // ambient occlusion

        double i_or = 2.42; // like diamond for crystal
    };

    explicit GGXMaterial(const shared_ptr<Texture>& albedo);
    explicit GGXMaterial(Params params);

    MaterialType type() const override
    {
        return MaterialType::GGX;
    }

    Color albedo(const HitRecord& rec) const override;
    Color emission(const HitRecord& rec) const override;

    double roughness(const HitRecord& rec) const;
    double metallic(const HitRecord& rec) const;

    dvec3 normal(const HitRecord& rec) const override;

    Color ambient_occlusion(const HitRecord& rec) const;
    Color evaluate(const HitRecord& rec,
                   const dvec3& wo,
                   const dvec3& wi) const override;

    MaterialSample sample(const HitRecord& rec,
                          const dvec3& wo) const override;

    double pdf(const HitRecord& rec,
               const dvec3& wo,
               const dvec3& wi) const override;

private:
    Params m_params;
};
