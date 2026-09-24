#include "scene/camera.h"

#include <cmath>
#include <random>
#include <glm/glm.hpp>

#include "core/constants.h"

Camera::Camera(const glm::dvec3& eye,
               const glm::dvec3& target,
               const glm::dvec3& up,
               double verticalFovDegrees,
               int imageWidth,
               int imageHeight,
               double aperture,
               double focusDist)
    : m_eye(eye),
      m_lensRadius(aperture * 0.5),
      m_focusDist(focusDist),
      m_imageWidth(imageWidth),
      m_imageHeight(imageHeight)
{
    m_forward = glm::normalize(target - eye);
    m_right = glm::normalize(glm::cross(m_forward, up));
    m_up = glm::normalize(glm::cross(m_right, m_forward));

    const double aspect = static_cast<double>(imageWidth) / imageHeight;
    const double theta = verticalFovDegrees * constants::kPi / 180.0;

    m_halfHeight = std::tan(theta * 0.5);
    m_halfWidth = aspect * m_halfHeight;
}

glm::dvec2 Camera::sampleUnitDisk()
{
    // Thread-local so this is safe if rays are generated across multiple threads.
    thread_local std::mt19937 rng{std::random_device{}()};
    thread_local std::uniform_real_distribution<double> dist(-1.0, 1.0);

    glm::dvec2 p;
    do
    {
        p = {dist(rng), dist(rng)};
    }
    while (glm::dot(p, p) >= 1.0);
    return p;
}

Ray Camera::generateRay(const double sampleX, const double sampleY) const
{
    // Map pixel coords to [-1, 1]
    const double u = (2.0 * sampleX / m_imageWidth) - 1.0;
    const double v = 1.0 - (2.0 * sampleY / m_imageHeight);

    // Direction through the pixel on the (unscaled) image plane at z=1.
    const glm::dvec3 dir =
        m_forward
        + u * m_halfWidth * m_right
        + v * m_halfHeight * m_up;

    if (m_lensRadius <= 0.0)
    {
        // Pinhole path, unchanged.
        return Ray{m_eye, glm::normalize(dir)};
    }

    // Point on the focal plane this pixel maps to.
    const glm::dvec3 focalPoint = m_eye + m_focusDist * glm::normalize(dir);

    // Random offset on the lens disk, in camera space.
    const glm::dvec2 lensSample = sampleUnitDisk() * m_lensRadius;
    const glm::dvec3 origin = m_eye + lensSample.x * m_right + lensSample.y * m_up;

    return Ray{origin, glm::normalize(focalPoint - origin)};
}

Ray Camera::generateRay(const int px, const int py) const
{
    return generateRay(px + 0.5, py + 0.5);
}
