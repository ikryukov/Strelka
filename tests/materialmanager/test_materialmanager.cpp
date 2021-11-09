#include "materialmanager.h"
#include "shadermanager/ShaderManager.h"
#include <doctest.h>

using namespace nevk;

TEST_CASE("material manager test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);
}

TEST_CASE("init MDL test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);

    std::string path = "/Users/jswark/Desktop/school/NeVKmain/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res = matmngr->initMDL(path.c_str());
    CHECK(res != 0);
}

TEST_CASE("load material test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);

    std::string pathso = "/Users/jswark/Desktop/school/NeVKmain/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res1 = matmngr->initMDL(pathso.c_str());
    CHECK(res1 != 0);

    std::string pathmdl = "/Users/jswark/Desktop/school/NeVKmain/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    bool res2 = matmngr->initMaterial(pathmdl.c_str());
    CHECK(res2 != 0);
}

TEST_CASE("compile material test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);

    std::string pathso = "/Users/jswark/Desktop/school/NeVKmain/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res1 = matmngr->initMDL(pathso.c_str());
    CHECK(res1 != 0);

    std::string pathmdl = "/Users/jswark/Desktop/school/NeVKmain/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    bool res2 = matmngr->initMaterial(pathmdl.c_str());
    CHECK(res2 != 0);

    std::string pathmdl1 = "gun_metal"; // todo: fix path
    std::string identifier = "gun_metal"; // material identifier
    bool res3 = matmngr->compileMaterial(pathmdl1, identifier);
    CHECK(res3 != 0);
}

TEST_CASE("code gen test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);

    std::string pathso = "/Users/jswark/Desktop/school/NeVKmain/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res1 = matmngr->initMDL(pathso.c_str());
    CHECK(res1 != 0);

    std::string pathmdl = "/Users/jswark/Desktop/school/NeVKmain/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    bool res2 = matmngr->initMaterial(pathmdl.c_str());
    CHECK(res2 != 0);

    std::string pathmdl1 = "gun_metal"; // todo: fix path
    std::string identifier = "gun_metal"; // material identifier (in file export material gun_metal)
    bool res3 = matmngr->compileMaterial(pathmdl1, identifier);
    CHECK(res3 != 0);

    std::string hlslCode;
    int id = 0;
    bool res4 = matmngr->initCodeGen();
    CHECK(res4 != 0);
    bool res5 = matmngr->translate(hlslCode);
    CHECK(res5 != 0);

    //ShaderManager::compileShader(const char* fileName, const char* entryPointName, Stage stage)
    nevk::ShaderManager* sm = new nevk::ShaderManager();
    uint32_t pixelShaderId = sm->loadShader("../../shaders/test/test_shader.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);
    CHECK(pixelShaderId != -1);
    CHECK(sm->loadShader("../../shaders/test/test_shader.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex) != -1);

    //std::cout << hlslCode << std::endl;
}