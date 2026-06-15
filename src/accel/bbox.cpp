// Minimal bounding box implementation.

#include "accel/bbox.h"


// ── Constructors ──────────────────────────────────────────────────────────────

BBox::BBox()
    : m_min( std::numeric_limits<double>::max())
    , m_max(-std::numeric_limits<double>::max())
{}

BBox::BBox(const glm::dvec3& minCorner, const glm::dvec3& maxCorner)
    : m_min(minCorner)
    , m_max(maxCorner)
{}

// ── maxExtentAxis ─────────────────────────────────────────────────────────────

int BBox::maxExtentAxis() const
{
    const glm::dvec3 d = m_max - m_min;
    if (d.x >= d.y && d.x >= d.z) return 0;
    if (d.y >= d.z) return 1;

    return 2;
}

// ── surfaceArea ───────────────────────────────────────────────────────────────

double BBox::surfaceArea() const
{
    glm::dvec3 d = m_max - m_min;
    return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
}

// ── unite ─────────────────────────────────────────────────────────────────────

// Expand to contain another box.
BBox BBox::unite(const BBox& a, const BBox& b)
{
    return BBox(glm::min(a.m_min, b.m_min),
                glm::max(a.m_max, b.m_max));
}

// Expand to contain a point.
BBox BBox::unite(const BBox& a, const glm::dvec3& p)
{
    return BBox(glm::min(a.m_min, p),
                glm::max(a.m_max, p));
}

// ── intersect ────────────────────────────────────────────────────────────────
// Slab method — works for all ray directions including axis-aligned.
// Returns true if the ray segment [tMin, tMax] overlaps the box.

bool BBox::intersect(const Ray& ray, double tMin, double tMax) const
{
    for (int axis = 0; axis < 3; ++axis)
    {
        const double invDir = 1.0 / ray.direction[axis];
        double t0 = (m_min[axis] - ray.origin[axis]) * invDir;
        double t1 = (m_max[axis] - ray.origin[axis]) * invDir;

        if (invDir < 0.0) std::swap(t0, t1);

        tMin = t0 > tMin ? t0 : tMin;
        tMax = t1 < tMax ? t1 : tMax;

        if (tMax < tMin) return false;
    }
    return true;
}
