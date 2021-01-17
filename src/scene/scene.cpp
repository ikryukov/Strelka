#include "scene.h"

namespace nevk
{
uint32_t Scene::createMesh(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib)
{
    Mesh mesh = {};
    mesh.mIndex = mIndices.size();
    mesh.mCount = ib.size();

    // adjust indices for global index buffer
    const uint32_t ibOffset = mIndices.size();
    for (int i = 0; i < ib.size(); ++i)
    {
        mIndices.push_back(ibOffset + ib[i]);
    }
    // copy vertices
    mVertices.insert(mVertices.end(), vb.begin(), vb.end());

    // add mesh to storage
    mMeshes.push_back(mesh);

    const uint32_t meshId = mMeshes.size() - 1;
    return meshId;
}

uint32_t Scene::createInstance(const uint32_t meshId, const uint32_t materialId, const glm::mat4& transform)
{
    assert(meshId < mMeshes.size());
    // assert(materialId < mMaterials.size());
    Instance inst = {};
    inst.mMaterialId = materialId;
    inst.mMeshId = meshId;
    inst.transform = transform;

    mInstances.push_back(inst);

    const uint32_t instId = mInstances.size() - 1;
    return instId;
}

} // namespace nevk