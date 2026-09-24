// Stub BVH interface kept in the starter for Direction 1.

#pragma once

#include <vector>
#include <string>

#include "bbox.h"
#include "accel/accel_structure.h"
#include "geometry/primitive.h"

class BVH : public AccelStructure
{
public:
    void build(const std::vector<std::shared_ptr<Primitive>>& primitives);
    bool intersect(const Ray& ray, HitRecord& rec) const override;
    const BBox& bounds() const { return m_nodes[0].bounds; }
    void dumpWireframeObj(const std::string& path, int maxDepth = -1) const;

private:
    struct BVHNode
    {
        BBox bounds;
        int left = -1;
        int right = -1;
        int primStart = -1;
        int primCount = 0;
    };

    int buildRecursive(int primStart, int primCount, int depth);
    [[nodiscard]] bool isLeaf(const BVHNode& node) const;
    bool intersectNode(int nodeIdx, const Ray& ray,
                       double tMin, double tMax, HitRecord& rec) const;

    // ← this line specifically
    void dumpNodeRecursive(int nodeIdx, std::ofstream& out, int& vertexOffset,
                           int maxDepth, int depth) const;

    std::vector<BVHNode> m_nodes;
    std::vector<std::shared_ptr<Primitive>> m_prims;

    static constexpr int kMaxDepth = 32;
    static constexpr int kMaxLeafPrims = 4;
};
