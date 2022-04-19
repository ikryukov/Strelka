#include "scene.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <algorithm>
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;

namespace oka
{

uint32_t Scene::createMesh(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib)
{
    std::scoped_lock lock(mMeshMutex);

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

uint32_t Scene::createInstance(const uint32_t meshId,
                               const uint32_t materialId,
                               const glm::mat4& transform,
                               const glm::float3& massCenter,
                               uint32_t lightId)
{
    assert(meshId < mMeshes.size());

    std::scoped_lock lock(mInstanceMutex);

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
    inst->lightId = lightId;

    mOpaqueInstances.push_back(instId);

    return instId;
}

uint32_t Scene::addMaterial(const MaterialDescription& material)
{
    // TODO: fix here
    uint32_t res = mMaterialsDescs.size();
    mMaterialsDescs.push_back(material);
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

uint32_t Scene::createLightMesh()
{
    std::vector<Scene::Vertex> vb;
    Scene::Vertex v1, v2, v3, v4;
    v1.pos = glm::float4(0.5f, 0.5f, 0.0f, 1.0f); // top right 0
    v2.pos = glm::float4(-0.5f, 0.5f, 0.0f, 1.0f); // top left 1
    v3.pos = glm::float4(-0.5f, -0.5f, 0.0f, 1.0f); // bottom left 2
    v4.pos = glm::float4(0.5f, -0.5f, 0.0f, 1.0f); // bottom right 3
    glm::float3 normal = glm::float3(1.f, 0.f, 0.f);
    v1.normal = v2.normal = v3.normal = v4.normal = packNormals(normal);
    std::vector<uint32_t> ib = { 0, 1, 2, 2, 3, 0 };
    vb.push_back(v1);
    vb.push_back(v2);
    vb.push_back(v3);
    vb.push_back(v4);

    uint32_t meshId = createMesh(vb, ib);
    assert(meshId != -1);

    return meshId;
}

glm::float3 computeFaceNormal(Scene::Vertex v1, Scene::Vertex v2, Scene::Vertex v3)
{
    const float EPSILON = 0.000001f;

    // default return value (0, 0, 0)
    glm::float3 n = glm::float3(0.f);

    // find 2 edge vectors: v1-v2, v1-v3
    float ex1 = v2.pos[0] - v1.pos[0];
    float ey1 = v2.pos[1] - v1.pos[1];
    float ez1 = v2.pos[2] - v1.pos[2];
    float ex2 = v3.pos[0] - v1.pos[0];
    float ey2 = v3.pos[1] - v1.pos[1];
    float ez2 = v3.pos[2] - v1.pos[2];

    // cross product: e1 x e2
    float nx, ny, nz;
    nx = ey1 * ez2 - ez1 * ey2;
    ny = ez1 * ex2 - ex1 * ez2;
    nz = ex1 * ey2 - ey1 * ex2;

    // normalize only if the length is > 0
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length > EPSILON)
    {
        // normalize
        float lengthInv = 1.0f / length;
        n[0] = nx * lengthInv;
        n[1] = ny * lengthInv;
        n[2] = nz * lengthInv;
    }

    return n;
}

uint32_t Scene::createSphereLightMesh()
{
    std::vector<Scene::Vertex> vertices(12); // 12 vertices
    std::vector<uint32_t> indices;

    const float PI = acos(-1);
    const float H_ANGLE = PI / 180 * 72; // 72 degree = 360 / 5
    const float V_ANGLE = atanf(1.0f / 2); // elevation = 26.565 degree

    int i1, i2; // indices
    float z, xy; // coords
    float hAngle1 = -PI / 2 - H_ANGLE / 2; // start from -126 deg at 2nd row
    float hAngle2 = -PI / 2; // start from -90 deg at 3rd row

    // the first top vertex (0, 0, r)
    float radius = 1.0f;
    Scene::Vertex v0, v1, v2, prevV;
    v0.pos = glm::float4(0, 0, radius, 1.f);

    glm::float3 normal = glm::float3(0.f, 0.f, 1.f);
    v0.normal = packNormals(normal);

    vertices[0] = v0;
    indices.push_back(0);
    prevV = v0;
    // 10 vertices at 2nd and 3rd rows
    for (int i = 1; i <= 5; ++i)
    {
        i1 = i;
        i2 = (i + 5);

        z = radius * sinf(V_ANGLE); // elevaton
        xy = radius * cosf(V_ANGLE);

        v1.pos = { xy * cosf(hAngle1), xy * sinf(hAngle1), z };
        v2.pos = { xy * cosf(hAngle2), xy * sinf(hAngle2), -z };
        v1.normal = v2.normal = packNormals(computeFaceNormal(prevV, v1, v2));
        vertices[i1] = v1;
        vertices[i2] = v2;

        indices.push_back(i1);
        indices.push_back(i2);

        // next horizontal angles
        hAngle1 += H_ANGLE;
        hAngle2 += H_ANGLE;

        prevV = v1;
    }

    // the last bottom vertex (0, 0, -r)
    normal = glm::float3(0.f, 0.f, -1.f);
    v1.normal = packNormals(normal);
    v1.pos = { 0, 0, -radius };
    i1 = 11;
    vertices[i1] = v1;
    indices.push_back(i1);

    uint32_t meshId = createMesh(vertices, indices);
    assert(meshId != -1);

    return meshId;
}

uint32_t Scene::createHardCodedSphere()
{

    const float N = 0.f;
    float phi = (1.0f + sqrt(5.0f)) * 0.5f; // golden ratio
    const float X = 1.0f;
    const float Z = 1.0f / phi;

    std::vector<Scene::Vertex> vertices;
    Scene::Vertex v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12;
    v1.pos = { -X, N, Z };
    v2.pos = { X, N, Z };
    v3.pos = { -X, N, -Z };
    v4.pos = { X, N, -Z };
    v5.pos = { N, Z, X };
    v6.pos = { N, Z, -X };
    v7.pos = { N, -Z, X };
    v8.pos = { N, -Z, -X };
    v9.pos = { Z, X, N };
    v10.pos = { -Z, X, N };
    v11.pos = { Z, -X, N };
    v12.pos = { -Z, -X, N };
    std::vector<uint32_t> indices = { 0,  4, 1, 0, 9, 4, 9, 5,  4, 4, 5,  8,  4,  8, 1, 8,  10, 1,  8, 3,
                                      10, 5, 3, 8, 5, 2, 3, 2,  7, 3, 7,  10, 3,  7, 6, 10, 7,  11, 6, 11,
                                      0,  6, 0, 1, 6, 6, 1, 10, 9, 0, 11, 9,  11, 2, 9, 2,  5,  7,  2, 11 };

    v1.normal = v5.normal = v2.normal = packNormals(computeFaceNormal(v1, v5, v2));
    v10.normal = packNormals(computeFaceNormal(v1, v10, v5));
    v6.normal = packNormals(computeFaceNormal(v10, v6, v5));
    v9.normal = packNormals(computeFaceNormal(v5, v6, v9));
    v2.normal = packNormals(computeFaceNormal(v5, v9, v2));
    v11.normal = packNormals(computeFaceNormal(v9, v11, v2));
    v4.normal = packNormals(computeFaceNormal(v9, v4, v11));
    v3.normal = packNormals(computeFaceNormal(v6, v3, v4));
    v7.normal = packNormals(computeFaceNormal(v1, v2, v7));
    v8.normal = packNormals(computeFaceNormal(v8, v11, v4));

    //    static const TriangleList triangles=
    //        {
    //            {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
    //            {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
    //            {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
    //            {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
    //        };

    vertices = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12 };

    uint32_t meshId = createMesh(vertices, indices);
    assert(meshId != -1);

    return meshId;
}
uint32_t Scene::createDiscLightMesh()
{
    std::vector<Scene::Vertex> vertices;
    std::vector<uint32_t> indices;

    Scene::Vertex v1, v2;
    v1.pos = glm::float4(0.f, 0.f, 0.f, 1.f);
    v2.pos = glm::float4(1.0f, 0.f, 0.f, 1.f);

    glm::float3 normal = glm::float3(0.f, 0.f, 1.f);
    v1.normal = v2.normal = packNormals(normal);

    vertices.push_back(v1); // central point
    vertices.push_back(v2); // first point

    const float diskRadius = 1.0f; // param
    const float step = 2.0f * M_PI / 16;
    float angle = 0;
    for (int i = 0; i < 16; ++i)
    {
        indices.push_back(0); // each triangle have central point
        indices.push_back(vertices.size() - 1); // prev vertex

        angle += step;
        const float x = cos(angle) * diskRadius;
        const float y = sin(angle) * diskRadius;

        Scene::Vertex v;
        v.pos = glm::float4(x, y, 0.0f, 1.0f);
        v.normal = packNormals(normal);
        vertices.push_back(v);

        indices.push_back(vertices.size() - 1); // added vertex
    }

    uint32_t meshId = createMesh(vertices, indices);
    assert(meshId != -1);

    return meshId;
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
                        float floatRotation[4] = { (float)sampler.outputsVec4[i][3], (float)sampler.outputsVec4[i][0],
                                                   (float)sampler.outputsVec4[i][1], (float)sampler.outputsVec4[i][2] };
                        float floatRotation1[4] = { (float)sampler.outputsVec4[i + 1][3],
                                                    (float)sampler.outputsVec4[i + 1][0],
                                                    (float)sampler.outputsVec4[i + 1][1],
                                                    (float)sampler.outputsVec4[i + 1][2] };
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

uint32_t Scene::createLight(const UniformLightDesc& desc)
{
    uint32_t lightId = (uint32_t)mLights.size();
    Light l;
    mLights.push_back(l);
    mLightDesc.push_back(desc);

    updateLight(lightId, desc);

    // TODO: only for rect light
    // Lazy init light mesh
    glm::float4x4 scaleMatrix = glm::float4x4(0.f);
    uint32_t currentLightId = 0;
    if (mRectLightMeshId == -1 && desc.type == 0)
    {
        mRectLightMeshId = createLightMesh();
        currentLightId = mRectLightMeshId;
        scaleMatrix = glm::scale(glm::float4x4(1.0f), glm::float3(desc.width, desc.height, 1.0f));
    }
    else if (mDiskLightMeshId == -1 && desc.type == 1)
    {
        mDiskLightMeshId = createDiscLightMesh();
        currentLightId = mDiskLightMeshId;
        scaleMatrix = glm::scale(glm::float4x4(1.0f), glm::float3(desc.radius, desc.radius, desc.radius));
    }
    else if (mSphereLightMeshId == -1 && desc.type == 2)
    {
        mSphereLightMeshId = createHardCodedSphere();
        currentLightId = mSphereLightMeshId;
        scaleMatrix = glm::scale(glm::float4x4(1.0f), glm::float3(desc.radius, desc.radius, desc.radius));
    }

    const glm::float4x4 transform = desc.useXform ? desc.xform * scaleMatrix : getTransform(desc);
    uint32_t instId = createInstance(currentLightId, (uint32_t)-1, transform, desc.position, lightId);
    assert(instId != -1);

    mLightIdToInstanceId[lightId] = instId;

    return lightId;
}

void Scene::updateLight(const uint32_t lightId, const UniformLightDesc& desc)
{
    // transform to GPU light
    if (desc.type == 0)
    {
        const glm::float4x4 scaleMatrix = glm::scale(glm::float4x4(1.0f), glm::float3(1.0f, desc.width, desc.height));
        const glm::float4x4 localTransform = desc.useXform ? scaleMatrix * desc.xform : getTransform(desc);

        mLights[lightId].points[1] = localTransform * glm::float4(-0.5f, 0.5f, 0.0f, 1.0f);
        mLights[lightId].points[2] = localTransform * glm::float4(-0.5f, -0.5f, 0.0f, 1.0f);
        mLights[lightId].points[0] = localTransform * glm::float4(0.5f, 0.5f, 0.0f, 1.0f);
        mLights[lightId].points[3] = localTransform * glm::float4(0.5f, -0.5f, 0.0f, 1.0f);

        mLights[lightId].type = 0;
    }
    else if (desc.type == 1)
    {
        const glm::float4x4 scaleMatrix =
            glm::scale(glm::float4x4(1.0f), glm::float3(desc.radius, desc.radius, desc.radius));
        const glm::float4x4 localTransform = desc.useXform ? desc.xform * scaleMatrix : getTransform(desc);

        mLights[lightId].points[0] = glm::float4(desc.radius, 0.f, 0.f, 0.f); // save radius
        mLights[lightId].points[1] = localTransform * glm::float4(0.f, 0.f, 0.f, 1.f); // save O
        mLights[lightId].points[2] = localTransform * glm::float4(1.f, 0.f, 0.f, 0.f); // OXws
        mLights[lightId].points[3] = localTransform * glm::float4(0.f, 1.f, 0.f, 0.f); // OYws

        glm::float4 normal = localTransform * glm::float4(0, 0, 1.f, 0.0f);
        mLights[lightId].normal = normal;
        mLights[lightId].type = 1;
    }
    else if (desc.type == 2)
    {
        const glm::float4x4 scaleMatrix =
            glm::scale(glm::float4x4(1.0f), glm::float3(desc.radius, desc.radius, desc.radius));
        const glm::float4x4 localTransform = desc.useXform ? scaleMatrix * desc.xform : getTransform(desc);

        mLights[lightId].points[0] = glm::float4(desc.radius, 0.f, 0.f, 0.f); // save radius
        mLights[lightId].points[1] = localTransform * glm::float4(0.f, 0.f, 0.f, 1.f); // save O

        glm::float4 normal = localTransform * glm::float4(0, 0, 1.f, 0.0f);
        mLights[lightId].normal = normal;

        mLights[lightId].type = 2;
    }

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

} // namespace oka
