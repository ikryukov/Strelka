#include "scene.h"

namespace nevk
{

uint32_t Scene::createMesh(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib)
{
    Mesh* mesh = nullptr;
    uint32_t meshId = -1;
    if (mDelMesh.empty())
    {
        meshId = mMeshes.size(); // add mesh to storage
        mMeshes.push_back({});
        mesh = &mMeshes.back();
    }
    else
    {
        meshId = mDelMesh.top(); // get index from stack
        mDelMesh.pop(); // del taken index from stack
        mesh = &mMeshes[meshId];
    }

    mesh->mIndex = mIndices.size(); // Index of 1st index in index buffer
    mesh->mCount = ib.size(); // amount of indices in mesh

    const uint32_t ibOffset = mIndices.size();  // adjust indices for global index buffer
    for (int i = 0; i < ib.size(); ++i)
    {
        mIndices.push_back(ibOffset + ib[i]);
    }
    mVertices.insert(mVertices.end(), vb.begin(), vb.end());  // copy vertices
    return meshId;
}

glm::mat4 Scene::createMeshTransform()
{
    // create the identity matrix needed for the subsequent matrix operations
    glm::mat4 identity{ 1.0f };
    // define the position, rotation axis, scale and how many degrees to rotate about the rotation axis.
    glm::float3 position{ 0.0f, 0.0f, 0.0f };
    glm::float3 rotationAxis{ 0.0f, 1.0f, 0.0f };
    glm::float3 scale{ 1.0f, 1.0f, 1.0f };
    float rotationDegrees{ 45.0f };

    /*  Transform matrix is calculated by:
    *
    *  Translating from the identity to the position, multiplied by
    *  Rotating from the identity about the rotationAxis by the rotationDegrees amount in radians, multiplied by
    *  Scaling from the identity by the scale vector.
    */
    return glm::translate(identity, position) *
           glm::rotate(identity, glm::radians(rotationDegrees), rotationAxis) *
           glm::scale(identity, scale);
}

uint32_t Scene::createInstance(const uint32_t meshId, const uint32_t materialId, const glm::mat4& transform)
{
    Instance* inst = nullptr;
    uint32_t instId = -1;
    if (mDelInstances.empty())
    {
        instId = mInstances.size();  // add instance to storage
        mInstances.push_back({});
        inst = &mInstances.back();
    }
    else
    {
        instId = mDelInstances.top(); // get index from stack
        mDelInstances.pop(); // del taken index from stack
        inst = &mInstances[instId];
    }
    inst->mMaterialId = materialId;
    inst->mMeshId = meshId;
    inst->transform = transform;

    return instId;
}

uint32_t Scene::createMaterial(const glm::float4& color)
{
    Material* material = nullptr;
    uint32_t materialId = -1;
    if (mDelMaterial.empty())
    {
        materialId = mMaterials.size(); // add material to storage
        mMaterials.push_back({});
        material = &mMaterials.back();
    }
    else
    {
        materialId = mDelMaterial.top(); // get index from stack
        mDelMaterial.pop(); // del taken index from stack
        material = &mMaterials[materialId];
    }
    material->color = color;
    return materialId;
}

void Scene::removeInstance(const uint32_t instId)
{
    mDelInstances.push(instId); // marked as removed
}

void Scene::removeMesh(const uint32_t meshId)
{
    mDelMesh.push(meshId); // marked as removed
}

void Scene::removeMaterial(const uint32_t materialId)
{
    mDelMaterial.push(materialId); // marked as removed
}

void Scene::updateInstanceTransform(uint32_t instId, glm::mat4 newTransform)
{
    Instance& inst = mInstances[instId];
    inst.transform = newTransform;
    mDirtyInstances.insert(instId);
}

void Scene::beginFrame()
{
    fr_mod = true;
    mDirtyInstances.clear();
}

void Scene::endFrame()
{
    fr_mod = false;
}
} // namespace nevk