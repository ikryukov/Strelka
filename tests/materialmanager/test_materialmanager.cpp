//#include "materialmanager.h"
#include "mdlHlslCodeGen.h"
#include "mdlMaterialCompiler.h"
#include "mdlNeurayLoader.h"
#include "mdlRuntime.h"
#include "mtlxMdlCodeGen.h"
#include "shadermanager/ShaderManager.h"

#include <doctest.h>
#include <fstream>
#include <filesystem>

using namespace nevk;
namespace fs = std::filesystem;

/*
TEST_CASE("Init Neuray Loader Test")
{
    nevk::MdlNeurayLoader* neurayLoader = new nevk::MdlNeurayLoader();
    CHECK(neurayLoader != nullptr);

    std::string path = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res = neurayLoader->init(path.c_str());
    CHECK(res != 0);
}


TEST_CASE("load material test")
{
    nevk::MdlNeurayLoader* neurayLoader = new nevk::MdlNeurayLoader();
        CHECK(neurayLoader != nullptr);

    std::string path = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res = neurayLoader->init(path.c_str());
        CHECK(res != 0);

    std::string pathmdl = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
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
*/
/*
TEST_CASE("hlsl code gen test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);

    std::string pathso = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res1 = matmngr->initMDL(pathso.c_str());
    CHECK(res1 != 0);

    std::string pathmdl = "/Users/jswark/school/USD_Build/mdl/"; // todo: fix path
    std::string resourcePath = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/resources"; // todo: fix path
    bool res2 = matmngr->initMaterial(pathmdl.c_str(), resourcePath.c_str());
    CHECK(res2 != 0);

    std::string pathmdl1 = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    std::string identifier = "carbon_composite"; // material identifier (in file export material gun_metal)
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
*/
/*
TEST_CASE("mtlx to mdl code gen test")
{
    nevk::MaterialManager* matmngr = new nevk::MaterialManager();
    CHECK(matmngr != nullptr);

    std::string mtlxLibPath = "/Users/jswark/Downloads/MaterialX_MacOS_Xcode_11_Python37/libraries/";
    matmngr->initMtlx(mtlxLibPath.c_str());
    std::string mtlxMaterialPath = "/Users/jswark/Downloads/MaterialX_MacOS_Xcode_11_Python37/resources/Materials/TestSuite/pbrlib/surfaceshader/sheen.mtlx";
    std::string mdlSrc;
    std::string ident;
    matmngr->translate(mtlxMaterialPath.c_str(), mdlSrc, ident);

    std::cout << mdlSrc;

    std::string pathso = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    bool res1 = matmngr->initMDL(pathso.c_str());
    CHECK(res1 != 0);

    std::string pathmdl = "/Users/jswark/Downloads/MaterialX_MacOS_Xcode_11_Python37/mdl/"; // todo: fix path
    bool res2 = matmngr->initMaterial(pathmdl.c_str());
    CHECK(res2 != 0);

    bool res3 = matmngr->compileMaterial(mdlSrc, ident);
    CHECK(res3 != 0);

    std::string hlslCode;
    int id = 0;
    bool res4 = matmngr->initCodeGen();
    CHECK(res4 != 0);

    bool res5 = matmngr->translate(hlslCode);
    CHECK(res5 != 0);

    std::cout << hlslCode;
}
*/
/*
TEST_CASE("mtlx to mdl code gen test")
{
    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    std::string hlslFile = "material.hlsl";
    std::ofstream hlslMaterial(mdlFile.c_str());

    std::string mtlxLibPath = "/Users/jswark/school/USD_Build/libraries";
    nevk::MtlxMdlCodeGen* mtlxCodeGen = new nevk::MtlxMdlCodeGen(mtlxLibPath.c_str());
        CHECK(mtlxCodeGen != nullptr);

    std::string mtlxMaterialPath = "/Users/jswark/school/USD_Build/resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx";
    std::string mdlSrc;
    std::string ident;
    mtlxCodeGen->translate(mtlxMaterialPath.c_str(), mdlSrc, ident);
    mdlMaterial << mdlSrc;

    std::string pathso = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    nevk::MdlRuntime* runtime = new nevk::MdlRuntime();
    std::string pathmdl = "/Users/jswark/school/USD_Build/mdl/"; // todo: fix path
    bool res = runtime->init(pathso.c_str(), pathmdl.c_str());
        CHECK(res != 0);

    nevk::MdlMaterialCompiler* matCompiler = new nevk::MdlMaterialCompiler(*runtime);

    mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
    bool res1 = matCompiler->compileMaterial(mdlSrc, ident, compiledMaterial);

    nevk::MdlHlslCodeGen* codeGen = new MdlHlslCodeGen();

    std::string hlslCode;
    int id = 0;
    bool res2 = codeGen->init(*runtime);
        CHECK(res2 != 0);

    std::vector<const mi::neuraylib::ICompiled_material*> materials;
    materials.push_back(compiledMaterial.get()); // 1 material
    bool res3 = codeGen->translate(materials, hlslCode);
        CHECK(res3 != 0);

    hlslMaterial << hlslCode;
}*/


TEST_CASE("mtlx to mdl code gen test")
{
    fs::path cwd = fs::current_path();



    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    std::string hlslFile = "material.hlsl";
    std::ofstream hlslMaterial(hlslFile.c_str());

    std::string pathso = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    nevk::MdlRuntime* runtime = new nevk::MdlRuntime();
    //std::string pathmdl = "/Users/jswark/school/USD_Build/mdl/"; // todo: fix path
    std::string pathmdl = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    std::string resourcePath = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/resources"; // todo: fix path

    bool res = runtime->init(resourcePath.c_str(), pathso.c_str(), pathmdl.c_str());
        CHECK(res != 0);

    nevk::MdlMaterialCompiler* matCompiler = new nevk::MdlMaterialCompiler(*runtime);

    mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
    // mdl -> hlsl
    std::string mdlSrc = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    std::string ident =  "carbon_composite";
    //
    bool res1 = matCompiler->compileMaterial(mdlSrc, ident, compiledMaterial);

    nevk::MdlHlslCodeGen* codeGen = new MdlHlslCodeGen();

    std::string hlslCode;
    int id = 0;
    bool res2 = codeGen->init(*runtime);
        CHECK(res2 != 0);

    std::vector<const mi::neuraylib::ICompiled_material*> materials;
    materials.push_back(compiledMaterial.get()); // 1 material
    bool res3 = codeGen->translate(materials, hlslCode);
        CHECK(res3 != 0);

    hlslMaterial << hlslCode;
}
