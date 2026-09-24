// Simple pinhole camera used to generate primary rays.

#pragma once

#include <glm/glm.hpp>

#include "core/ray.h"
#pragma once

class Camera
{
public:
    Camera(const glm::dvec3& eye,
           const glm::dvec3& target,
           const glm::dvec3& up,
           double verticalFovDegrees,
           int imageWidth,
           int imageHeight,
           double aperture = 0.0, // NEW: lens diameter; 0 = pinhole
           double focusDist = 1.0); // NEW: distance to focal plane

    Ray generateRay(double sampleX, double sampleY) const;
    Ray generateRay(int px, int py) const;

    int imageWidth() const { return m_imageWidth; }
    int imageHeight() const { return m_imageHeight; }

private:
    glm::dvec3 m_eye;
    glm::dvec3 m_forward;
    glm::dvec3 m_right;
    glm::dvec3 m_up;

    double m_halfHeight;
    double m_halfWidth;

    double m_lensRadius;
    double m_focusDist;

    int m_imageWidth;
    int m_imageHeight;

    static glm::dvec2 sampleUnitDisk(); // NEW
};
