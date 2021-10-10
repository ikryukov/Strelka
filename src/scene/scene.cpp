#include "scene.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <algorithm>
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;

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

    mesh->mVertex = mVertices.size(); // Index of 1st vertex in vertex buffer
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
    inst->isLight = mMaterials[materialId].isLight;

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

std::string Scene::getSceneFileName()
{
    fs::path p(modelPath);
    return p.filename().string();
};

std::string Scene::getSceneDir()
{
    fs::path p(modelPath);
    return p.parent_path().string();
};

//  valid range of coordinates [-1; 1]
uint32_t packNormals(const glm::float3& normal)
{
    uint32_t packed = (uint32_t)((normal.x + 1.0f) / 2.0f * 511.99999f);
    packed += (uint32_t)((normal.y + 1.0f) / 2.0f * 511.99999f) << 10;
    packed += (uint32_t)((normal.z + 1.0f) / 2.0f * 511.99999f) << 20;
    return packed;
}

void Scene::createLightMesh()
{
    std::vector<Scene::Vertex> vb;
    Scene::Vertex v1, v2, v3, v4;
    v1.pos = glm::float4(0.0f, 0.5f, 0.5f, 1.0f); // top right 0
    v2.pos = glm::float4(0.0f, -0.5f, 0.5f, 1.0f); // top left 1
    v3.pos = glm::float4(0.0f, -0.5f, -0.5f, 1.0f); // bottom left 2
    v4.pos = glm::float4(0.0f, 0.5f, -0.5f, 1.0f); // bottom right 3
    glm::float3 normal = glm::float3(1.f, 0.f, 0.f);
    v1.normal = v2.normal = v3.normal = v4.normal = packNormals(normal);
    std::vector<uint32_t> ib = { 0, 1, 2, 2, 3, 0 };
    vb.push_back(v1);
    vb.push_back(v2);
    vb.push_back(v3);
    vb.push_back(v4);

    uint32_t meshId = createMesh(vb, ib);
    assert(meshId != -1);
}

void Scene::updateAnimation(const float time)
{
    if (mAnimations.empty())
    {
        return;
    }
    auto& animation = mAnimations[0];
    for (auto& channel : animation.channels)
    {
        assert(channel.node < mNodes.size());
        auto& sampler = animation.samplers[channel.samplerIndex];
        if (sampler.inputs.size() > sampler.outputsVec4.size())
        {
            continue;
        }
        for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
        {
            if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1]))
            {
                float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
                if (u <= 1.0f)
                {
                    switch (channel.path)
                    {
                    case AnimationChannel::PathType::TRANSLATION: {
                        glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
                        mNodes[channel.node].translation = glm::float3(trans);
                        break;
                    }
                    case AnimationChannel::PathType::SCALE: {
                        glm::vec4 scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
                        mNodes[channel.node].scale = glm::float3(scale);
                        break;
                    }
                    case AnimationChannel::PathType::ROTATION: {
                        float floatRotation[4] = {
                            (float)sampler.outputsVec4[i][3],
                            (float)sampler.outputsVec4[i][0],
                            (float)sampler.outputsVec4[i][1],
                            (float)sampler.outputsVec4[i][2]
                        };
                        float floatRotation1[4] = {
                            (float)sampler.outputsVec4[i + 1][3],
                            (float)sampler.outputsVec4[i + 1][0],
                            (float)sampler.outputsVec4[i + 1][1],
                            (float)sampler.outputsVec4[i + 1][2]
                        };
                        glm::quat q1 = glm::make_quat(floatRotation);
                        glm::quat q2 = glm::make_quat(floatRotation1);
                        mNodes[channel.node].rotation = glm::normalize(glm::slerp(q1, q2, u));
                        break;
                    }
                    }
                }
            }
        }
    }
    mCameras[0].matrices.view = getTransform(mCameras[0].node);
}

uint32_t Scene::createLight(const RectLightDesc& desc)
{
    const glm::float4x4 localTransform = getTransform(desc);

    Light l;
    l.points[0] = localTransform * glm::float4(0.0f, 0.5f, 0.5f, 1.0f);
    l.points[1] = localTransform * glm::float4(0.0f, -0.5f, 0.5f, 1.0f);
    l.points[2] = localTransform * glm::float4(0.0f, -0.5f, -0.5f, 1.0f);
    l.points[3] = localTransform * glm::float4(0.0f, 0.5f, -0.5f, 1.0f);
    l.color = glm::float4(desc.color, 1.0f) * desc.intensity;

    RectLightDesc lightDesc{};
    lightDesc.position = desc.position;
    lightDesc.orientation = desc.orientation;
    lightDesc.width = desc.width;
    lightDesc.height = desc.height;
    lightDesc.color = desc.color;
    lightDesc.intensity = desc.intensity;

    uint32_t lightId = (uint32_t)mLights.size();
    mLights.push_back(l);
    mLightDesc.push_back(desc);

    Material light;
    light.isLight = 1;
    light.baseColorFactor = glm::float4(desc.color, 1.0f);
    light.texBaseColor = -1;
    light.texMetallicRoughness = -1;
    light.roughnessFactor = 0.01;
    light.illum = 2;
    uint32_t matId = addMaterial(light);

    uint32_t instId = createInstance(0, matId, localTransform, desc.position);
    assert(instId != -1);

    mLightIdToInstanceId[lightId] = instId;

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
    mLights[lightId].color = glm::float4(desc.color, 1.0f) * desc.intensity;
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
