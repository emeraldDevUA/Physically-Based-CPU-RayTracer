// Light sources seen by the integrator.

#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "core/constants.h"
#include "shading/auxhiliary_methods.h"
#include "core/color.h"

class Light
{
public:
    virtual ~Light() = default;

    struct Sample
    {
        dvec3 position; // Point on the light surface
        dvec3 normal; // Surface normal at that point
        Color emission; // Emitted radiance
        double pdf; // Probability density of this sample
    };

    virtual Sample samplePoint(double u, double v) const = 0;
    virtual double pdf(const dvec3& point) const = 0;
};

class PointLight : public Light
{
public:
    PointLight(const dvec3& position,
               const Color& intensityColor,
               double intensity);

    const dvec3& position() const;
    Color intensity() const;

    Sample samplePoint(double /*u*/, double /*v*/) const override
    {
        return {m_position, dvec3(0.0), intensity(), 1.0};
    }

    double pdf(const dvec3&) const override
    {
        return 1.0;
    }

private:
    dvec3 m_position;
    Color m_intensityColor;
    double m_intensity;
};


class RectLight : public Light
{
public:
    RectLight(const dvec3& origin,
              const dvec3& edge1,
              const dvec3& edge2,
              const Color& emission,
              double intensity)
        : m_origin(origin),
          m_edge1(edge1),
          m_edge2(edge2),
          m_normal(glm::normalize(glm::cross(edge1, edge2))),
          m_emission(intensity * emission),
          m_area(glm::length(glm::cross(edge1, edge2)))
    {
    }

    Sample samplePoint(double u, double v) const override
    {
        return {m_origin + u * m_edge1 + v * m_edge2, m_normal, m_emission, 1.0 / m_area};
    }

    double pdf(const glm::dvec3&) const override
    {
        return 1.0 / m_area;
    }

private:
    dvec3 m_origin, m_edge1, m_edge2, m_normal;
    Color m_emission;
    double m_area;
};


class DiskLight : public Light
{
public:
    DiskLight(const dvec3& center,
              const dvec3& normal,
              double radius,
              const Color& emission,
              double intensity)
        : m_center(center),
          m_normal(glm::normalize(normal)),
          m_radius(radius),
          m_area(constants::kPi * radius * radius),
          m_emission(intensity * emission)
    {
        buildTBN(m_normal, m_tangent, m_bitangent);
    }

    Sample samplePoint(const double u, const double v) const override
    {
        const double r = m_radius * std::sqrt(u);
        const double theta = 2.0 * constants::kPi * v;
        const dvec3 local(r * std::cos(theta), r * std::sin(theta), 0.0);

        return {m_center + local.x * m_tangent + local.y * m_bitangent, m_normal, m_emission, 1.0 / m_area};
    }

    double pdf(const dvec3&) const override { return 1.0 / m_area; }

private:
    dvec3 m_center, m_normal, m_tangent, m_bitangent;
    double m_radius, m_area;
    Color m_emission;


};
