#include <scene/scene.h>

#include <doctest.h>

TEST_CASE("test checkBeginFrameStatus")
{
    auto* scene = new oka::Scene();
    scene->beginFrame();
    bool rez = scene->getFrMod();
    CHECK(rez == true);
}

TEST_CASE("test checkBeginFrameDirty")
{
    auto* scene = new oka::Scene();
    scene->beginFrame();
    CHECK(scene->getDirtyInstances().empty() == true);
}

TEST_CASE("test createMesh")
{
    oka::Scene scene;
    std::vector<oka::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);
    CHECK(meshId != -1);
}

TEST_CASE("test createMesh complex")
{
    oka::Scene scene;
    std::vector<oka::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshIdFst = scene.createMesh(vb, ib);
    uint32_t meshIdSnd = scene.createMesh(vb, ib);
    CHECK(meshIdFst != meshIdSnd);
    scene.removeMesh(meshIdFst);
    uint32_t meshIdThd = scene.createMesh(vb, ib);
    CHECK(meshIdFst == meshIdThd);
}

TEST_CASE("test createInstance")
{
    oka::Scene scene;
    std::vector<oka::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);

    oka::Scene::MaterialDescription currMaterial{};
    scene.addMaterial(currMaterial);
    uint32_t matId = 0;

    glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);
    for (const oka::Scene::Vertex& vertPos : vb)
    {
        sum += vertPos.pos;
    }
    glm::float3 massCenter = sum / (float)vb.size();

    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = scene.createInstance(meshId, matId, transform, massCenter);
    CHECK(instId != -1);
}

TEST_CASE("test createInstance complex")
{
    oka::Scene scene;
    std::vector<oka::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);

    oka::Scene::MaterialDescription currMaterial{};
    scene.addMaterial(currMaterial);
    uint32_t matId = 0;

    glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);
    for (const oka::Scene::Vertex& vertPos : vb)
    {
        sum += vertPos.pos;
    }
    glm::float3 massCenter = sum / (float)vb.size();

    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t firstId = scene.createInstance(meshId, matId, transform, massCenter);
    uint32_t secondId = scene.createInstance(meshId, matId, transform, massCenter);
    CHECK(firstId != secondId);
    scene.removeInstance(firstId);
    uint32_t thirdId = scene.createInstance(meshId, matId, transform, massCenter);
    CHECK(firstId == thirdId);
}

TEST_CASE("test checkMesh")
{
    oka::Scene scene;
    std::vector<oka::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);
    CHECK(meshId == 0);
    CHECK(scene.mMeshes.size() == 1);
}

TEST_CASE("test checkInstance")
{
    oka::Scene scene;
    std::vector<oka::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);

    oka::Scene::MaterialDescription currMaterial{};
    scene.addMaterial(currMaterial);
    uint32_t matId = 0;

    glm::float3 sum = glm::float3(0.0f, 0.0f, 0.0f);
    for (const oka::Scene::Vertex& vertPos : vb)
    {
        sum += vertPos.pos;
    }
    glm::float3 massCenter = sum / (float)vb.size();

    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = scene.createInstance(meshId, matId, transform, massCenter);
    CHECK(instId == 0);
    CHECK(scene.mInstances.size() == 1);
}

//TEST_CASE("test addMaterial")
//{
//    nevk::Scene scene;
//    Material currMaterial{};
//
//    currMaterial.diffuse = glm::float4(1.0f, 1.0f, 1.0f, 1.0f);
//    currMaterial.texNormalId = 1;
//    currMaterial.sampNormalId = 1;
//    currMaterial.baseColorFactor = glm::float4(1.0f, 1.0f, 1.0f, 1.0f);
//    currMaterial.texBaseColor = 1;
//    currMaterial.sampBaseId = 1;
//    currMaterial.roughnessFactor = (float)1;
//    currMaterial.metallicFactor = (float)1;
//    currMaterial.texMetallicRoughness = 1;
//    currMaterial.sampMetallicRoughness = 1;
//    currMaterial.emissiveFactor = glm::float3(1.0f, 1.0f, 1.0f);
//    currMaterial.texEmissive = 1;
//    currMaterial.sampEmissiveId = 1;
//    currMaterial.texOcclusion = 1;
//    currMaterial.sampOcclusionId = 1;
//    currMaterial.d = (float)0.1;
//    currMaterial.illum = 1;
//
//    scene.addMaterial(currMaterial);
//    CHECK(scene.mMaterials.size() == 1);
//}
