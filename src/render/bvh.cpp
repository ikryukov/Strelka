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

BvhBuilder::AABB BvhBuilder::computeCentroids(const std::vector<BvhNodeInternal>& nodes, uint32_t start, uint32_t end)
{
    AABB result;
    for (uint32_t i = start; i < end; ++i)
    {
        const BvhNodeInternal& n = nodes[i];
        result.expand(n.box.getCentroid());
    }
    return result;
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

    uint32_t mid = (begin + end) / 2;
    AABB centorids = computeCentroids(nodes, begin, end);
    const uint32_t axis = centorids.getMaxinumExtent();
    switch (mSplitMethod)
    {
    case SplitMethod::eMiddleBounds: {
        // method from ray tracing in one weekend
        AABB bounds = computeBounds(nodes, begin, end);
        const uint32_t splitAxis = bounds.getMaxinumExtent();
        std::sort(nodes.begin() + begin, nodes.begin() + end, [splitAxis](const BvhNodeInternal& a, const BvhNodeInternal& b) { 
            return a.box.minimum[splitAxis] < b.box.minimum[splitAxis]; });
        break;
    }
    case SplitMethod::eMiddleCentroids: {
        // The primitives are classified into the two sets, depending on whether their centroids are above or below the midpoint
        const float pmid = (centorids.minimum[axis] + centorids.maximum[axis]) * 0.5f;
        BvhNodeInternal* midPtr = std::partition(&nodes[begin], &nodes[end - 1] + 1, [axis, pmid](const BvhNodeInternal& ni) {
            return ni.box.getCentroid()[axis] < pmid;
        });
        // convert from pointers to indices
        mid = midPtr - &nodes[begin] + begin;
        if (mid != begin && mid != end)
            break;
        // in that case, execution falls through to the equals method
    }
    case SplitMethod::eEquals: {
        // partition into equally sized subsets
        mid = (begin + end) / 2;
        std::nth_element(&nodes[begin], &nodes[mid],
                         &nodes[end - 1] + 1,
                         [axis](const BvhNodeInternal& a, const BvhNodeInternal& b) {
                             return a.box.getCentroid()[axis] < b.box.getCentroid()[axis];
                         });
        break;
    }
    default:
        break;
    }

    uint32_t currentNodeId = (uint32_t)nodes.size();
    nodes.push_back(BvhNodeInternal());

    BvhNodeInternal currentNode;

    currentNode.isLeaf = false;

    currentNode.left = recursiveBuild(nodes, begin, mid);
    currentNode.right = recursiveBuild(nodes, mid, end);

    currentNode.box = AABB::Union(nodes[currentNode.left].box, nodes[currentNode.right].box);

    nodes[currentNodeId] = currentNode;

    return currentNodeId;
}

//std::vector<BVHNode> BvhBuilder::build(const std::vector<Scene::Vertex>& vertices, const std::vector<uint32_t>& indices)
// std::vector<BVHNode> BvhBuilder::build(const std::vector<glm::float3>& positions)
BVH BvhBuilder::build(const std::vector<glm::float3>& positions)
{
    const uint32_t totalTriangles = (uint32_t)positions.size() / 3;
    if (totalTriangles == 0)
    {
        return BVH();
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

    BVH res;
    res.nodes.resize(nodes.size());
    res.triangles.resize(totalTriangles);
    uint32_t triangleId = 0;
    for (uint32_t i = 0; i < (uint32_t)nodes.size(); ++i)
    {
        BvhNodeInternal& oldNode = nodes[i];
        BVHNode& newNode = res.nodes[oldNode.visitOrder];
        newNode.nodeOffset = oldNode.next == InvalidMask ? InvalidMask : nodes[oldNode.next].visitOrder;
        newNode.instId = oldNode.prim;
        if (oldNode.prim != InvalidMask) // leaf
        {
            newNode.minBounds = oldNode.triangle.v1 - oldNode.triangle.v0;
            newNode.maxBounds = oldNode.triangle.v2 - oldNode.triangle.v0;
            res.triangles[triangleId].v0 = glm::float4(oldNode.triangle.v0, 0.0f);
            newNode.instId = triangleId;
            ++triangleId;
        }
        else
        {
            newNode.minBounds = oldNode.box.minimum;
            newNode.maxBounds = oldNode.box.maximum;
        }
    }

    return res;
}

} // namespace nevk
