// Point light implementation.

#include "shading/light.h"

PointLight::PointLight(const dvec3& position,
                       const Color& intensityColor,
                       const double intensity)
    : m_position(position),
      m_intensityColor(intensityColor),
      m_intensity(intensity) {}

const dvec3& PointLight::position() const {
    return m_position;
}

Color PointLight::intensity() const {
    return m_intensity * m_intensityColor;
}
