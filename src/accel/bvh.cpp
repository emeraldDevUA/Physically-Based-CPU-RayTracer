// Starter stub for a BVH acceleration structure.

#include "accel/bvh.h"
#include <algorithm>
#include <fstream>
bool BVH::intersect(const Ray& ray, HitRecord& rec) const
{
    if (m_nodes.empty()) return false;
    return intersectNode(0, ray, ray.tMin, ray.tMax, rec);
}

void BVH::build(const std::vector<std::shared_ptr<Primitive>>& primitives)
{
    m_prims = primitives;
    m_nodes.clear();
    m_nodes.reserve(m_prims.size() * 2); // upper bound on node count

    buildRecursive(0, static_cast<int>(m_prims.size()), 0);
}

static BBox computeCentroidBounds(const std::vector<std::shared_ptr<Primitive>>& prims,
                                  const int start, const int count)
{
    BBox b;
    for (int i = start; i < start + count; ++i)
    {
        b = BBox::unite(b, prims[i]->bounds().centroid());
    }
        return b;
}

static BBox computeBounds(const std::vector<std::shared_ptr<Primitive>>& prims,
                          const int start, const int count)
{
    BBox b;
    for (int i = start; i < start + count; ++i)
        b = BBox::unite(b, prims[i]->bounds());
    return b;
}


int BVH::buildRecursive(int primStart, int primCount, int depth)
{
    int nodeIdx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(BVHNode{}); // may reallocate – use indices, not pointers
    BVHNode& node = m_nodes[nodeIdx]; // re-fetch after potential realloc below

    node.bounds = computeBounds(m_prims, primStart, primCount);

    node.primStart = primStart;
    node.primCount = primCount;

    if (primCount <= kMaxLeafPrims || depth >= kMaxDepth)
    {
        // left/right stay -1  →  isLeaf() returns true
        node.left = -1;
        node.right = -1;
        return nodeIdx;
    }

    BBox centBounds = computeCentroidBounds(m_prims, primStart, primCount);
    int axis = centBounds.maxExtentAxis(); // 0=x, 1=y, 2=z

    // If all centroids are identical on every axis, make a leaf.
    if (std::abs(centBounds.max()[axis] - centBounds.min()[axis]) < constants::kEpsilon)
    {
        return nodeIdx;
    }

    // ── SAH bucketing ─────────────────────────────────────────────────────
    constexpr int kBuckets = 32;
    struct Bucket
    {
        int count = 0;
        BBox bounds;
    };
    Bucket buckets[kBuckets];

    double span = centBounds.max()[axis] - centBounds.min()[axis];
    double invSpan = 1.0 / span;

    // Place each primitive into a bucket by centroid position.
    for (int i = primStart; i < primStart + primCount; ++i)
    {
        double c = m_prims[i]->bounds().centroid()[axis];
        int b = static_cast<int>(kBuckets * (c - centBounds.min()[axis]) * invSpan);
        b = glm::clamp(b, 0, kBuckets - 1);
        buckets[b].count++;
        buckets[b].bounds = BBox::unite(buckets[b].bounds, m_prims[i]->bounds());
    }

    double costs[kBuckets - 1];
    double invParentArea = 1.0 / m_nodes[nodeIdx].bounds.surfaceArea();

    for (int i = 0; i < kBuckets - 1; ++i)
    {
        BBox leftBounds, rightBounds;
        int leftCount = 0, rightCount = 0;

        for (int j = 0; j <= i; ++j)
        {
            leftBounds = BBox::unite(leftBounds, buckets[j].bounds);
            leftCount += buckets[j].count;

        }
        for (int j = i + 1; j < kBuckets; ++j)
        {
            rightBounds = BBox::unite(rightBounds, buckets[j].bounds);
            rightCount += buckets[j].count;
        }

        costs[i] = 0.125 + (leftCount * leftBounds.surfaceArea() +
            rightCount * rightBounds.surfaceArea()) * invParentArea;
    }

    // Find the lowest cost split.
    int bestSplit = 0;
    double bestCost = costs[0];
    for (int i = 1; i < kBuckets - 1; ++i)
    {
        if (costs[i] < bestCost)
        {
            bestCost = costs[i];
            bestSplit = i;
        }
    }

    // If a leaf is cheaper than the best split, make a leaf.
    if (bestCost >= primCount)
            return nodeIdx;

    // ── Partition ─────────────────────────────────────────────────────────
    auto* first = m_prims.data() + primStart;
    auto* last = first + primCount;
    auto* mid = std::partition(first, last,
                               [&](const std::shared_ptr<Primitive>& p)
                               {
                                   const double c = p->bounds().centroid()[axis];
                                   int b = static_cast<int>(kBuckets * (c - centBounds.min()[axis]) * invSpan);
                                   b = glm::clamp(b, 0, kBuckets - 1);
                                   return b <= bestSplit;
                               });

    int leftCount = static_cast<int>(mid - first);

    // Fallback: partition produced an empty side — median split instead.
    if (leftCount == 0 || leftCount == primCount)
    {
        std::sort(first, last, [axis](const auto& a, const auto& b)
        {
            return a->bounds().centroid()[axis] < b->bounds().centroid()[axis];
        });
        leftCount = primCount / 2;
    }

    // ── Recurse ───────────────────────────────────────────────────────────
    // Both children are built before writing back to m_nodes[nodeIdx]
    // because push_back inside the calls may reallocate the vector.
    int leftIdx = buildRecursive(primStart, leftCount, depth + 1);
    int rightIdx = buildRecursive(primStart + leftCount, primCount - leftCount, depth + 1);

    // Re-fetch after potential realloc.
    m_nodes[nodeIdx].left = leftIdx;
    m_nodes[nodeIdx].right = rightIdx;


    return nodeIdx;
}

bool BVH::isLeaf(const BVHNode& node) const
{
    return node.left == -1 && node.right == -1;
}

bool BVH::intersectNode(int nodeIdx, const Ray& ray,
                        double tMin, double tMax, HitRecord& rec) const
{
    const BVHNode& node = m_nodes[nodeIdx];

    // Ray–AABB slab test: skip if miss.
    if (!node.bounds.intersect(ray, tMin, tMax))
        return false;

    // ── Leaf: test all primitives ──────────────────────────────────────────
    if (isLeaf(node))
    {
        bool hit = false;
        for (int i = node.primStart; i < node.primStart + node.primCount; ++i)
        {
            HitRecord tmp;
            Ray newRay = Ray(ray.origin, ray.direction, tMin, tMax);
            if (m_prims[i]->intersect(newRay, tmp))
            {
                hit = true;
                tMax = tmp.t; // ← add this
                rec = tmp;
            }
        }
        return hit;
    }

    // ── Interior: visit nearer child first for early-exit ─────────────────
    int first = node.left;
    int second = node.right;

    // Heuristic: visit the child whose min-corner is closer along the ray axis.
    // A simple proxy: compare AABB centroids on the split axis.
    // (Full front-to-back ordering requires storing the split axis – omitted
    //  here for clarity; swapping based on ray direction sign also works.)

    glm::dvec3 c1 =
        (m_nodes[first].bounds.min() + m_nodes[first].bounds.max()) * 0.5;

    glm::dvec3 c2 =
        (m_nodes[second].bounds.min() + m_nodes[second].bounds.max()) * 0.5;

    double d1 = glm::dot(c1 - ray.origin, ray.direction);
    double d2 = glm::dot(c2 - ray.origin, ray.direction);

    if (d1 > d2)
        std::swap(first, second);

    HitRecord leftRec;
    bool hitL = intersectNode(first, ray, tMin, tMax, leftRec);
    if (hitL) tMax = leftRec.t; // ← shrink the window before visiting second child

    HitRecord rightRec;
    bool hitR = intersectNode(second, ray, tMin, tMax, rightRec);

    if (hitL && hitR)
    {
        rec = (leftRec.t < rightRec.t) ? leftRec : rightRec;
        return true;
    }
    if (hitL)
    {
        rec = leftRec;
        return true;
    }
    if (hitR)
    {
        rec = rightRec;
        return true;
    }
    return false;
}



void BVH::dumpWireframeObj(const std::string& path, int maxDepth) const {
    if (m_nodes.empty()) return;

    std::ofstream out(path);
    if (!out.is_open()) return;

    int vertexOffset = 0;
    dumpNodeRecursive(0, out, vertexOffset, maxDepth, 0);
}

void BVH::dumpNodeRecursive(int nodeIdx, std::ofstream& out, int& vertexOffset,
                             int maxDepth, int depth) const {
    if (nodeIdx < 0 || nodeIdx >= static_cast<int>(m_nodes.size())) return;
    if (maxDepth >= 0 && depth > maxDepth) return;

    const BVHNode& node = m_nodes[nodeIdx];
    const glm::dvec3 mn = node.bounds.min();   // ← call, not member access
    const glm::dvec3 mx = node.bounds.max();   // ← call, not member access

    const glm::dvec3 corners[8] = {
        {mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z}, {mx.x, mx.y, mn.z}, {mn.x, mx.y, mn.z},
        {mn.x, mn.y, mx.z}, {mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z}
    };
    for (const auto& c : corners)
        out << "v " << c.x << " " << c.y << " " << c.z << "\n";

    static constexpr int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    for (const auto& e : edges)
        out << "l " << (vertexOffset + e[0] + 1) << " " << (vertexOffset + e[1] + 1) << "\n";

    vertexOffset += 8;

    if (!isLeaf(node)) {
        dumpNodeRecursive(node.left,  out, vertexOffset, maxDepth, depth + 1);
        dumpNodeRecursive(node.right, out, vertexOffset, maxDepth, depth + 1);
    }
}