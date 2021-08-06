#include "bvh.h"


namespace nevk
{
BvhBuilder::BvhBuilder()
{
}

BvhBuilder::~BvhBuilder()
{
}

BvhBuilder::AABB BvhBuilder::computeBounds(const std::vector<BvhNodeInternal>& nodes, uint32_t start, uint32_t end)
{
    AABB result;
    for (uint32_t i = start; i < end; ++i)
    {
        const BvhNodeInternal& n = nodes[i];
        result.expand(n.box);
    }
    return result;
}

bool BvhBuilder::nodeCompare(const BvhNodeInternal& a, const BvhNodeInternal& b, const int axis)
{
    return a.box.minimum[axis] < b.box.minimum[axis];
}

bool BvhBuilder::nodeCompareX(const BvhNodeInternal& a, const BvhNodeInternal& b)
{
    return nodeCompare(a, b, 0);
}

bool BvhBuilder::nodeCompareY(const BvhNodeInternal& a, const BvhNodeInternal& b)
{
    return nodeCompare(a, b, 1);
}

bool BvhBuilder::nodeCompareZ(const BvhNodeInternal& a, const BvhNodeInternal& b)
{
    return nodeCompare(a, b, 2);
}

void BvhBuilder::setDepthFirstVisitOrder(std::vector<BvhNodeInternal>& nodes, uint32_t nodeId, uint32_t nextId, uint32_t& order)
{
    BvhNodeInternal& node = nodes[nodeId];
    node.visitOrder = order++;
    node.next = nextId;
    if (node.left != InvalidMask)
    {
        setDepthFirstVisitOrder(nodes, node.left, node.right, order);
    }
    if (node.right != InvalidMask)
    {
        setDepthFirstVisitOrder(nodes, node.right, nextId, order);
    }
}

void BvhBuilder::setDepthFirstVisitOrder(std::vector<BvhNodeInternal>& nodes, uint32_t root)
{
    uint32_t order = 0;
    setDepthFirstVisitOrder(nodes, root, InvalidMask, order);
}

uint32_t BvhBuilder::recursiveBuild(std::vector<BvhNodeInternal>& nodes, uint32_t begin, uint32_t end)
{
    uint32_t count = end - begin;
    if (count == 1)
    {
        return begin;
    }
    AABB bounds = computeBounds(nodes, begin, end);
    uint32_t splitAxis = rand() % 3;
    auto comparator = (splitAxis == 0) ? nodeCompareX :
                      (splitAxis == 1) ? nodeCompareY :
                                         nodeCompareZ;

    std::sort(nodes.begin() + begin, nodes.begin() + end, comparator);

    uint32_t mid = (begin + end) / 2;
    uint32_t currentNodeId = (uint32_t)nodes.size();
    nodes.push_back(BvhNodeInternal());

    BvhNodeInternal currentNode;
    currentNode.box = bounds;
    currentNode.isLeaf = false;

    currentNode.left = recursiveBuild(nodes, begin, mid);
    currentNode.right = recursiveBuild(nodes, mid, end);

    nodes[currentNodeId] = currentNode;

    return currentNodeId;
}

//std::vector<BVHNode> BvhBuilder::build(const std::vector<Scene::Vertex>& vertices, const std::vector<uint32_t>& indices)
std::vector<BVHNode> BvhBuilder::build(const std::vector<glm::float3>& positions)
{
    const uint32_t totalTriangles = (uint32_t)positions.size() / 3;
    if (totalTriangles == 0)
    {
        return std::vector<BVHNode>();
    }
    std::vector<BvhNodeInternal> nodes(totalTriangles);
    // need to apply transform
    for (uint32_t i = 0; i < totalTriangles; ++i)
    {
        BvhNodeInternal& leaf = nodes[i];
        leaf.prim = i;
        leaf.isLeaf = true;
        leaf.triangle.v0 = positions[i * 3 + 0];
        leaf.triangle.v1 = positions[i * 3 + 1];
        leaf.triangle.v2 = positions[i * 3 + 2];
        leaf.box = boundingBox(leaf.triangle);
    }

    uint32_t root = recursiveBuild(nodes, 0, totalTriangles);

    setDepthFirstVisitOrder(nodes, root);

    std::vector<BVHNode> res(nodes.size());

    for (uint32_t i = 0; i < (uint32_t)nodes.size(); ++i)
    {
        BvhNodeInternal& oldNode = nodes[i];
        BVHNode& newNode = res[oldNode.visitOrder];
        if (oldNode.prim != InvalidMask) // leaf
        {
            newNode.minBounds = oldNode.triangle.v1 - oldNode.triangle.v0;
            newNode.maxBounds = oldNode.triangle.v2 - oldNode.triangle.v0;
            newNode.v = oldNode.triangle.v0;
            newNode.nodeOffset = oldNode.next == InvalidMask ? InvalidMask : nodes[oldNode.next].visitOrder;
        }
        else
        {
            newNode.instId = oldNode.prim;
            newNode.nodeOffset = oldNode.next == InvalidMask ? InvalidMask : nodes[oldNode.next].visitOrder;
            newNode.minBounds = oldNode.box.minimum;
            newNode.maxBounds = oldNode.box.maximum;
        }
    }

    return res;
}

} // namespace nevk
