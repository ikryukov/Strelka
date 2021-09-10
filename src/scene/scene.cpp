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

uint32_t Scene::addMaterial(const Material& material)
{
    // TODO: fix here
    uint32_t res = mMaterials.size();
    mMaterials.push_back(material);
    return res;
}

glm::float4x4 getTransform(const Scene::RectLightDesc& desc)
{
    const glm::float4x4 translationMatrix = glm::translate(glm::float4x4(1.0f), desc.position);
    glm::quat rotation = glm::quat(glm::radians(desc.orientation)); // to quaternion
    const glm::float4x4 rotationMatrix{ rotation };
    glm::float3 scale = {1.0f, desc.width, desc.height};
    const glm::float4x4 scaleMatrix = glm::scale(glm::float4x4(1.0f), scale);

    const glm::float4x4 localTransform = translationMatrix * rotationMatrix * scaleMatrix;

    return localTransform;
}

uint32_t Scene::createLight(const RectLightDesc& desc)
{
    const glm::float4x4 localTransform = getTransform(desc);

    Light l;
    l.points[0] = localTransform * glm::float4(0.0f, 0.5f, 0.5f, 1.0f);
    l.points[1] = localTransform * glm::float4(0.0f, -0.5f, 0.5f, 1.0f);
    l.points[2] = localTransform * glm::float4(0.0f, -0.5f, -0.5f, 1.0f);
    l.points[3] = localTransform * glm::float4(0.0f, 0.5f, -0.5f, 1.0f);
    l.color = glm::float4(desc.color, 1.0f);

    RectLightDesc lightDesc;
    lightDesc.position = desc.position;
    lightDesc.orientation = desc.orientation;
    lightDesc.width = desc.width;
    lightDesc.height = desc.height;
    lightDesc.color = desc.color;

    uint32_t lightId = (uint32_t)mLights.size();
    mLights.push_back(l);
    mLightDesc.push_back(desc);

    return lightId;
}


void Scene::updateLight(const uint32_t lightId, const RectLightDesc& desc)
{
    const glm::float4x4 localTransform = getTransform(desc);

    // transform to GPU light
    mLights[lightId].points[0] = localTransform * glm::float4(0.0f, 0.5f, 0.5f, 1.0f);
    mLights[lightId].points[1] = localTransform * glm::float4(0.0f, -0.5f, 0.5f, 1.0f);
    mLights[lightId].points[2] = localTransform * glm::float4(0.0f, -0.5f, -0.5f, 1.0f);
    mLights[lightId].points[3] = localTransform * glm::float4(0.0f, 0.5f, -0.5f, 1.0f);
    mLights[lightId].color = glm::float4(desc.color, 1.0f);
}

void Scene::removeInstance(const uint32_t instId)
{
    mDelInstances.push(instId); // marked as removed
}

void Scene::removeLight(const uint32_t lightId)
{
    mLights.erase(mLights.begin() + lightId);
    mLightDesc.erase(mLightDesc.begin() + lightId);
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
