#pragma once
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>

#include <algorithm>
#include <scene.h>
#include <vector>


namespace nevk
{

// GPU structure
struct BVHNode
{
    glm::float3 minBounds;
    int instId;
    glm::float3 maxBounds;
    int nodeOffset;
};

struct BVHTriangle
{
    glm::float4 v0;
};

struct BVH
{
    std::vector<BVHNode> nodes;
    std::vector<BVHTriangle> triangles;
};

class BvhBuilder
{
public:

    enum class SplitMethod: uint32_t
    {
        eMiddleCentroids,
        eMiddleBounds,
        eEquals
    };

    BvhBuilder();
    ~BvhBuilder();

    //std::vector<BVHNode> build(const std::vector<Scene::Vertex>& vertices, const std::vector<uint32_t>& indices);
    // std::vector<BVHNode> build(const std::vector<glm::float3>& positions);
    BVH build(const std::vector<glm::float3>& positions);

private:
    SplitMethod mSplitMethod = SplitMethod::eMiddleCentroids;

    struct AABB
    {
        glm::float3 minimum{ 1e10f };
        glm::float3 maximum{ -1e10f };
        AABB()
        {
        }
        AABB(const glm::float3& a, const glm::float3& b)
        {
            minimum = a;
            maximum = b;
        }

        static AABB Union(const AABB& a, const AABB& b)
        {
            AABB res;
            res.expand(a);
            res.expand(b);
            return res;
        }

        glm::float3 getCentroid() const
        {
            return (minimum + maximum) * 0.5f;
        }
        
        // returns number of axis
        uint32_t getMaxinumExtent()
        {
            uint32_t res = 2;
            glm::float3 dim = maximum - minimum;
            if (dim.x > dim.y && dim.x > dim.z)
            {
                res = 0;
            }
            else if (dim.y > dim.x && dim.y > dim.z)
            {
                res = 1;
            }
            return res;
        }

        void expand(const glm::float3& p)
        {
            minimum.x = std::min(minimum.x, p.x);
            minimum.y = std::min(minimum.y, p.y);
            minimum.z = std::min(minimum.z, p.z);

            maximum.x = std::max(maximum.x, p.x);
            maximum.y = std::max(maximum.y, p.y);
            maximum.z = std::max(maximum.z, p.z);
        }

        void expand(const AABB& o)
        {
            minimum.x = std::min(minimum.x, o.minimum.x);
            minimum.y = std::min(minimum.y, o.minimum.y);
            minimum.z = std::min(minimum.z, o.minimum.z);

            maximum.x = std::max(maximum.x, o.maximum.x);
            maximum.y = std::max(maximum.y, o.maximum.y);
            maximum.z = std::max(maximum.z, o.maximum.z);
        }
    };

    struct Triangle
    {
        glm::float3 v0;
        glm::float3 v1;
        glm::float3 v2;
    };

    AABB boundingBox(const Triangle& tri)
    {
        float minX = std::min(tri.v0.x, std::min(tri.v1.x, tri.v2.x));
        float minY = std::min(tri.v0.y, std::min(tri.v1.y, tri.v2.y));
        float minZ = std::min(tri.v0.z, std::min(tri.v1.z, tri.v2.z));

        float maxX = std::max(tri.v0.x, std::max(tri.v1.x, tri.v2.x));
        float maxY = std::max(tri.v0.y, std::max(tri.v1.y, tri.v2.y));
        float maxZ = std::max(tri.v0.z, std::max(tri.v1.z, tri.v2.z));

        const float eps = 1e-6f;
        // need to pad aabb to prevent from ultra thin box (zero width)
        return AABB(glm::float3(minX, minY, minZ), glm::float3(maxX + eps, maxY + eps, maxZ + eps));
    }

    static const uint32_t LeafMask = 0x80000000;
    static const uint32_t InvalidMask = 0xFFFFFFFF;

    struct BvhNodeInternal
    {
        BvhNodeInternal(){};
        AABB box;
        bool isLeaf = false;
        uint32_t visitOrder = InvalidMask;
        uint32_t next = InvalidMask;
        uint32_t prim = InvalidMask;
        uint32_t left = InvalidMask;
        uint32_t right = InvalidMask;
        Triangle triangle; // only for leaf
    };

    static bool nodeCompare(const BvhNodeInternal& a, const BvhNodeInternal& b, const int axis);
    static bool nodeCompareX(const BvhNodeInternal& a, const BvhNodeInternal& b);
    static bool nodeCompareY(const BvhNodeInternal& a, const BvhNodeInternal& b);
    static bool nodeCompareZ(const BvhNodeInternal& a, const BvhNodeInternal& b);

    void setDepthFirstVisitOrder(std::vector<BvhNodeInternal>& nodes, uint32_t nodeId, uint32_t nextId, uint32_t& order);
    void setDepthFirstVisitOrder(std::vector<BvhNodeInternal>& nodes, uint32_t root);

    AABB computeBounds(const std::vector<BvhNodeInternal>& nodes, uint32_t start, uint32_t end);
    AABB computeCentroids(const std::vector<BvhNodeInternal>& nodes, uint32_t start, uint32_t end);
    uint32_t recursiveBuild(std::vector<BvhNodeInternal>& nodes, uint32_t begin, uint32_t end);
};

} // namespace nevk
