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
    nevk::Mesh* mesh = new nevk::Mesh();
    CHECK(mesh != nullptr);
}

TEST_CASE("test createInstance")
{
    nevk::Instance* instance = new nevk::Instance();
    CHECK(instance != nullptr);
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