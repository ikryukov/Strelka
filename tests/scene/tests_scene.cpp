#include <scene/scene.h>

#include <doctest.h>

TEST_CASE("test checkBeginFrameStatus")
{
    auto* scene = new nevk::Scene();
    scene->beginFrame();
    bool rez = scene->getFrMod();
    CHECK(rez == true);
}

TEST_CASE("test checkBeginFrameDirty")
{
    auto* scene = new nevk::Scene();
    scene->beginFrame();
    CHECK(scene->getDirtyInstances().empty() == true);
}

TEST_CASE("test createMesh")
{
    nevk::Scene scene;
    std::vector<nevk::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);
    CHECK(meshId != -1);
}

TEST_CASE("test createMesh complex")
{
    nevk::Scene scene;
    std::vector<nevk::Scene::Vertex> vb;
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
    nevk::Scene scene;
    std::vector<nevk::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);
    uint32_t matId = scene.createMaterial(glm::float4(1.0));
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = scene.createInstance(meshId, matId, transform);
    CHECK(instId != -1);
}

TEST_CASE("test createInstance complex")
{
    nevk::Scene scene;
    std::vector<nevk::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);
    uint32_t matId = scene.createMaterial(glm::float4(1.0));
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t firstId = scene.createInstance(meshId, matId, transform);
    uint32_t secondId = scene.createInstance(meshId, matId, transform);
    CHECK(firstId != secondId);
    scene.removeInstance(firstId);
    uint32_t thirdId = scene.createInstance(meshId, matId, transform);
    CHECK(firstId == thirdId);
}

TEST_CASE("test createMaterial")
{
    nevk::Scene scene;
    uint32_t matId = scene.createMaterial(glm::float4(1.0));
    CHECK(matId != -1);
}

TEST_CASE("test createMaterial complex")
{
    nevk::Scene scene;

    uint32_t matIdFst = scene.createMaterial(glm::float4(1.0));
    uint32_t matIdSnd = scene.createMaterial(glm::float4(1.0));
    CHECK(matIdFst != matIdSnd);
    scene.removeMaterial(matIdFst);
    uint32_t matIdThd = scene.createMaterial(glm::float4(1.0));
    CHECK(matIdFst == matIdThd);
}


TEST_CASE("test checkMesh")
{
    nevk::Scene scene;
    std::vector<nevk::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);
    CHECK(meshId == 0);
    CHECK(scene.mMeshes.size() == 1);
}

TEST_CASE("test checkInstance")
{
    nevk::Scene scene;
    std::vector<nevk::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);
    uint32_t matId = scene.createMaterial(glm::float4(1.0));
    glm::float4x4 transform{ 1.0f };
    glm::translate(transform, glm::float3(0.0f, 0.0f, 0.0f));
    uint32_t instId = scene.createInstance(meshId, matId, transform);
    CHECK(instId == 0);
    CHECK(scene.mInstances.size() == 1);
}

TEST_CASE("test checkMaterial")
{
    nevk::Scene scene;
    uint32_t matId = scene.createMaterial(glm::float4(1.0));
    CHECK(matId == 0);
    CHECK(scene.mMaterials.size() == 1);
}
