#include "scene.h"

namespace nevk
{

uint32_t Scene::createMesh(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib)
{
    if (mDelMesh.empty())
    {
        Mesh mesh = {};
        mesh.mIndex = mIndices.size(); // Index of 1st index in index buffer
        mesh.mCount = ib.size(); // amount of indices in mesh

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
    else
    {
        uint32_t meshIndex = mDelMesh.top(); // get index from stack
        mDelMesh.pop(); // del taken index from stack
        Mesh mesh = mMeshes[meshIndex];
        mesh.mIndex = mIndices.size(); // Index of 1st index in index buffer
        mesh.mCount = ib.size(); // amount of indices in mesh

        // adjust indices for global index buffer
        const uint32_t ibOffset = mIndices.size();
        for (int i = 0; i < ib.size(); ++i)
        {
            mIndices.push_back(ibOffset + ib[i]);
        }
        // copy vertices
        mVertices.insert(mVertices.end(), vb.begin(), vb.end());

        mMeshes[meshIndex] = mesh; // update old Mesh to a new one
        return meshIndex;
    }
}

void MeshInstance::init_rotateBy(const float& degrees)
{
    this->rotateBy(degrees);
}

glm::mat4 MeshInstance::getTransformMatrix() const
{
    return this->transformMatrix;
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
    assert(meshId < mMeshes.size());
    assert(materialId < mMaterials.size());

    if (mDelInstances.empty())
    {
        Instance inst = {};
        inst.mMaterialId = materialId;
        inst.mMeshId = meshId;
        inst.transform = transform;

        mInstances.push_back(inst);

        const uint32_t instId = mInstances.size() - 1;
        return instId;
    }
    else
    {
        uint32_t instIndex = mDelInstances.top(); // get index from stack
        mDelInstances.pop(); // del taken index from stack
        Instance inst = mInstances[instIndex];
        inst.mMaterialId = materialId;
        inst.mMeshId = meshId;
        inst.transform = transform;
        mInstances[instIndex] = inst; // update old Instance to a new one
        return instIndex; // ?
    }
}

void Scene::createMaterial(const glm::float4& color)
{
    if (mDelMaterial.empty())
    {
        Material mater = {};
        mater.color = color;
        mMaterials.push_back(mater);
    }
    else
    {
        uint32_t materIndex = mDelMaterial.top(); // get index from stack
        mDelMaterial.pop(); // del taken index from stack
        Material mater = mMaterials[materIndex];
        mater.color = color;
        mMaterials[materIndex] = mater; // update old Material to a new one
    }
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

bool Vertex::operator==(const Vertex& other) const
{
    return pos == other.pos && uv == other.uv;
}

} // namespace nevk
