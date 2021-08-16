#include "scene.h"

#include <glm/gtx/norm.hpp>

#include <algorithm>
#include <utility>

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

    const uint32_t ibOffset = mVertices.size(); // adjust indices for global index buffer
    for (int i = 0; i < ib.size(); ++i)
    {
        mIndices.push_back(ibOffset + ib[i]);
    }
    mVertices.insert(mVertices.end(), vb.begin(), vb.end()); // copy vertices
    return meshId;
}

uint32_t Scene::createInstance(const uint32_t meshId, const uint32_t materialId, const glm::mat4& transform, const glm::float3& massCenter)
{
    Instance* inst = nullptr;
    uint32_t instId = -1;
    if (mDelInstances.empty())
    {
        instId = mInstances.size(); // add instance to storage
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
    inst->massCenter = massCenter;

    if (mMaterials[materialId].isTransparent())
    {
        mTransparentInstances.push_back(instId);
    }
    else
    {
        mOpaqueInstances.push_back(instId);
    }

    return instId;
}


uint32_t Scene::createMaterial(const glm::float4& ambient,
                               const glm::float4& diffuse,
                               const glm::float4& specular,
                               const glm::float4& emissive,
                               const glm::float4& transparency,
                               float opticalDensity,
                               float shininess,
                               uint32_t illum,
                               uint32_t texAmbientId,
                               uint32_t texDiffuseId,
                               uint32_t texSpecularId,
                               uint32_t texNormalId,
                               float d)
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
    material->ambient = ambient;
    material->diffuse = diffuse;
    material->specular = specular;
    material->emissive = emissive;
    material->transparency = transparency;
    material->opticalDensity = opticalDensity;
    material->shininess = shininess;
    material->illum = illum;
    material->texAmbientId = texAmbientId;
    material->texDiffuseId = texDiffuseId;
    material->texSpecularId = texSpecularId;
    material->texNormalId = texNormalId;
    material->d = d;

    return materialId;
}

uint32_t Scene::addMaterial(const Material& material)
{
    // TODO: fix here
    uint32_t res = mMaterials.size();
    mMaterials.push_back(material);
    return res;
}

uint32_t Scene::createLight(const glm::float3& v0, const glm::float3& v1, const glm::float3& v2)
{
    Light l;
    l.v0 = v0;
    l.v1 = v1;
    l.v2 = v2;

    uint32_t lightId = (uint32_t) mLights.size();
    mLights.push_back(l);
    return lightId;
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

std::vector<uint32_t>& Scene::getOpaqueInstancesToRender(const glm::float3& camPos)
{
    sort(mOpaqueInstances.begin(), mOpaqueInstances.end(),
         [&camPos, this](const uint32_t& instId1, const uint32_t& instId2) {
             return glm::distance2(camPos, getInstances()[instId1].massCenter) <
                    glm::distance2(camPos, getInstances()[instId2].massCenter);
         });

    return mOpaqueInstances;
}

std::vector<uint32_t>& Scene::getTransparentInstancesToRender(const glm::float3& camPos)
{
    sort(mTransparentInstances.begin(), mTransparentInstances.end(),
         [&camPos, this](const uint32_t& instId1, const uint32_t& instId2) {
             return glm::distance2(camPos, getInstances()[instId1].massCenter) >
                    glm::distance2(camPos, getInstances()[instId2].massCenter);
         });

    return mTransparentInstances;
}

std::set<uint32_t> Scene::getDirtyInstances()
{
    return this->mDirtyInstances;
}

bool Scene::getFrMod()
{
    return this->FrMod;
}

void Scene::updateInstanceTransform(uint32_t instId, glm::float4x4 newTransform)
{
    Instance& inst = mInstances[instId];
    inst.transform = newTransform;
    mDirtyInstances.insert(instId);
}

void Scene::beginFrame()
{
    FrMod = true;
    mDirtyInstances.clear();
}

void Scene::endFrame()
{
    FrMod = false;
}

} // namespace nevk
