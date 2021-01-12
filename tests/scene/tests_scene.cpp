#include <scene/scene.h>

#include <doctest.h>


TEST_CASE("test sceneCreation")
{
    nevk::Scene* scene = new nevk::Scene();
    CHECK(scene != nullptr);
}

TEST_CASE("test checkBeginFrameStatus")
{
    nevk::Scene* scene = new nevk::Scene();
    scene->beginFrame();
    bool rez = scene->fr_mod;
    CHECK(rez == true);
}

TEST_CASE("test checkBeginFrameDirty")
{
    nevk::Scene* scene = new nevk::Scene();
    scene->beginFrame();
    CHECK(scene->mDirtyInstances.empty() == true); // true
}

TEST_CASE("test createMesh")
{
    nevk::Scene scene;
    std::vector<nevk::Scene::Vertex> vb;
    std::vector<uint32_t> ib;
    uint32_t meshId = scene.createMesh(vb, ib);

    CHECK(meshId != -1);
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
    nevk::Material* material = new nevk::Material();
    CHECK(material != nullptr);
}

//TEST_CASE("test checkMesh")
//{
//    nevk::Scene* scene = new nevk::Scene();
//    nevk::Mesh m = {};
//    nevk::Mesh* mesh = new nevk::Mesh();
//    m = *mesh;
//    scene->mMeshes.push_back(m);
//
//    CHECK(scene->mMeshes[0] == *mesh);
//}
//
//TEST_CASE("test checkInstance")
//{
//    nevk::Scene* scene = new nevk::Scene();
//    nevk::Instance inst = {};
//    nevk::Instance* instance = new nevk::Instance();
//    inst = *instance;
//    scene->mInstances.push_back(inst);
//    CHECK( == nullptr);
//}
//
//TEST_CASE("test checkMaterial")
//{
//    nevk::Scene* scene = new nevk::Scene();
//    nevk::Material m = {};
//    nevk::Material* material = new nevk::Material();
//    m = *material;
//    scene->mMaterials.push_back(m);
//
//        CHECK( == nullptr);
//}

/*
 * создается сцена, меш и инстанс, проверяется что он есть и у инстанса нужный меш выставлен
 */
//TEST_CASE("test checkMeshInst")
//{
//    auto* scene = new nevk::Scene();
//
//    nevk::Mesh m = {};
//    auto* mesh = new nevk::Mesh();
//    m = *mesh;
//    scene->mMeshes.push_back(m);
//
//    nevk::Instance inst = {};
//    auto* instance = new nevk::Instance();
//    inst = *instance;
//    inst.mMeshId = mesh->mIndex;
//    scene->mInstances.push_back(inst);
//
//    CHECK();
//}
//
//TEST_CASE("test removeInstance")
//{
//    nevk::Scene* scene = new nevk::Scene();
//    nevk::Instance inst = {};
//    nevk::Instance* instance = new nevk::Instance();
//    inst = *instance;
//    scene->mInstances.push_back(inst);
//    scene->removeInstance();
//    CHECK( == nullptr);
//}
//
//TEST_CASE("test removeMesh")
//{
//    nevk::Scene* scene = new nevk::Scene();
//    nevk::Mesh m = {};
//    nevk::Mesh* mesh = new nevk::Mesh();
//    m = *mesh;
//    scene->mMeshes.push_back(m);
//    scene->removeMesh();
//    CHECK( == nullptr);
//}