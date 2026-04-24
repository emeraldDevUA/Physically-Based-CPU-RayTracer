// Implementation of the minimal pinhole camera.

#include "scene/camera.h"

#include <cmath>
#include <glm/glm.hpp>

#include "core/constants.h"

Camera::Camera(const glm::dvec3& eye,
               const glm::dvec3& target,
               const glm::dvec3& up,
               double verticalFovDegrees,
               int imageWidth,
               int imageHeight)
    : m_eye(eye),
      m_imageWidth(imageWidth),
      m_imageHeight(imageHeight) {
    m_forward = glm::normalize(target - eye);
    m_right = glm::normalize(glm::cross(m_forward, up));
    m_up = glm::normalize(glm::cross(m_right, m_forward));

    const double aspect = static_cast<double>(imageWidth) / imageHeight;
    const double theta = verticalFovDegrees * constants::kPi / 180.0;

    m_halfHeight = std::tan(theta * 0.5);
    m_halfWidth = aspect * m_halfHeight;
}

Ray Camera::generateRay(const double sampleX, const double sampleY) const {
    // Map pixel coords to [-1, 1]
    const double u = (2.0 * sampleX / m_imageWidth)  - 1.0;
    const double v = 1.0 - (2.0 * sampleY / m_imageHeight); // flip Y: pixel Y grows down, view Y grows up

    const glm::dvec3 dir =
        m_forward
        + u * m_halfWidth  * m_right
        + v * m_halfHeight * m_up;

    return Ray{m_eye, glm::normalize(dir)};
}
Ray Camera::generateRay(const int px, const int py) const {
    return generateRay(px + 0.5, py + 0.5);
}
