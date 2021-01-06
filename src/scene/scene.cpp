#include "scene.h"

namespace nevk
{


uint32_t Scene::createMesh(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib)
{
    Mesh mesh = {};
    mesh.mIndex = mIndices.size(); // Index of 1st index in index buffer // 1st vertex ??
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

void MeshInstance::init_update(const glm::mat4& projectionViewMatrix)
{
    this->update(projectionViewMatrix);
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

//////////////////////////////////////////////////////////////////////////

//void Scene::update_scene(const float& delta) {
////    const glm::mat4 cameraMatrix{camera.getProjectionMatrix() * camera.getViewMatrix()};
//
//    for (auto& mesh : mMeshes) {
//        mesh.rotateBy(delta * 45.0f);
//        mesh.update(cameraMatrix);
//    }
//}
//////////////////////////////////////////////////////////////////////////

uint32_t Scene::createInstance(const uint32_t meshId, const uint32_t materialId, const glm::mat4& transform)
{
    assert(meshId < mMeshes.size());
    assert(materialId < mMaterials.size());
    Instance inst = {};
    inst.mMaterialId = materialId;
    inst.mMeshId = meshId;
    inst.transform = transform;

    mInstances.push_back(inst);

    const uint32_t instId = mInstances.size() - 1;
    return instId;
}

void Scene::createMaterial(const glm::float4& color)
{
    Material mater = {};
    mater.color = color;

    mMaterials.push_back(mater);
}

void Scene::removeInstance(const uint32_t instId)
{
    mInstances.erase(mInstances.begin() + instId);
}

void Scene::removeMesh(const uint32_t meshId)
{
    mMeshes.erase(mMeshes.begin() + meshId);
}

void Scene::removeMaterial(const uint32_t materialId)
{
    mMaterials.erase(mMaterials.begin() + materialId);
}


bool Vertex::operator==(const Vertex& other) const
{
    return pos == other.pos && uv == other.uv;
}

} // namespace nevk