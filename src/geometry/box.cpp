// Oriented (non-axis-aligned) bounding box intersection implementation.

#include "geometry/box.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

Box::Box(const dvec3& center,
         const glm::dquat& rotation,
         const dvec3& halfExtents,
         std::shared_ptr<Material> material)
    : m_center(center),
      m_rotation(glm::normalize(rotation)),
      m_halfExtents(halfExtents),
      m_material(std::move(material)) {}


BBox Box::bounds() const
{
    // Transform all eight local corners into world space and take their
    // axis-aligned extent, since BBox itself is axis-aligned.
    dvec3 worldMin(std::numeric_limits<double>::max());
    dvec3 worldMax(std::numeric_limits<double>::lowest());

    for (int i = 0; i < 8; ++i) {
        const dvec3 local(
            (i & 1) ? m_halfExtents.x : -m_halfExtents.x,
            (i & 2) ? m_halfExtents.y : -m_halfExtents.y,
            (i & 4) ? m_halfExtents.z : -m_halfExtents.z);

        const dvec3 world = m_center + m_rotation * local;
        worldMin = glm::min(worldMin, world);
        worldMax = glm::max(worldMax, world);
    }

    return {worldMin, worldMax};
}


bool Box::intersect(const Ray& ray, HitRecord& rec) const {
    // Transform the ray into the box's local (axis-aligned) space. Since
    // m_rotation is a unit quaternion, its conjugate is its inverse.
    const glm::dquat invRotation = glm::conjugate(m_rotation);
    const dvec3 localOrigin = invRotation * (ray.origin - m_center);
    const dvec3 localDir    = invRotation * ray.direction;

    double tMin = ray.tMin;
    double tMax = ray.tMax;

    int entryAxis = -1;
    double entrySign = 1.0;
    int exitAxis = -1;
    double exitSign = 1.0;

    for (int axis = 0; axis < 3; ++axis) {
        const double invD = 1.0 / localDir[axis];

        double t0 = (-m_halfExtents[axis] - localOrigin[axis]) * invD;
        double t1 = ( m_halfExtents[axis] - localOrigin[axis]) * invD;

        // Outward normal points along -axis if we enter through the near
        // (min) face, +axis if we enter through the far (max) face.
        double sign = -1.0;
        if (invD < 0.0) {
            std::swap(t0, t1);
            sign = 1.0;
        }

        if (t0 > tMin) {
            tMin = t0;
            entryAxis = axis;
            entrySign = sign;
        }
        if (t1 < tMax) {
            tMax = t1;
            exitAxis = axis;
            exitSign = -sign; // the exit face is the opposite face
        }

        if (tMax <= tMin) {
            return false;
        }
    }

    // If no face tightened tMin, every entry (near) face was behind the ray
    // origin — i.e. the ray starts inside the box. This happens for rays
    // continuing through a dielectric interior, so fall back to the exit
    // (far) face instead of reporting a miss.
    int hitAxis;
    double hitSign;
    double t;
    if (entryAxis != -1) {
        t = tMin;
        hitAxis = entryAxis;
        hitSign = entrySign;
    } else if (exitAxis != -1) {
        t = tMax;
        hitAxis = exitAxis;
        hitSign = exitSign;
    } else {
        return false;
    }

    rec.t = t;
    rec.position = ray.at(rec.t);

    dvec3 localNormal(0.0);
    localNormal[hitAxis] = hitSign;
    const dvec3 outwardNormal = m_rotation * localNormal;
    rec.setFaceNormal(ray.direction, outwardNormal);
    rec.material = m_material;

    // Planar per-face UV mapping using the two non-hit local axes.
    const dvec3 localHit = localOrigin + localDir * rec.t;
    const int u = (hitAxis + 1) % 3;
    const int v = (hitAxis + 2) % 3;
    rec.uv = glm::dvec2(
        0.5 * (localHit[u] / m_halfExtents[u] + 1.0),
        0.5 * (localHit[v] / m_halfExtents[v] + 1.0));

    return true;
}