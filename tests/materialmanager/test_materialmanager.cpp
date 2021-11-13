#include "materialmanager.h"
#include "mdlHlslCodeGen.h"
#include "mdlMaterialCompiler.h"
#include "mdlNeurayLoader.h"
#include "mdlRuntime.h"
#include "mtlxMdlCodeGen.h"
#include "shadermanager/ShaderManager.h"

#include <render/render.h>

#include <doctest.h>
#include <fstream>

using namespace nevk;

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
    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    std::string hlslFile = "material.hlsl";
    std::ofstream hlslMaterial(hlslFile.c_str());

    std::string pathso = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib"; // todo: fix path
    std::string pathmdl = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    std::string resourcePath = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/resources"; // todo: fix path

    nevk::MdlRuntime* runtime = new nevk::MdlRuntime();
    bool res = runtime->init(resourcePath.c_str(), pathso.c_str(), pathmdl.c_str());
    CHECK(res != 0);

    nevk::MdlMaterialCompiler* matCompiler = new nevk::MdlMaterialCompiler(*runtime);

    mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
    // mdl -> hlsl
    std::string mdlSrc = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // todo: fix path
    std::string ident = "carbon_composite";
    //
    bool res1 = matCompiler->compileMaterial(mdlSrc, ident, compiledMaterial);

    std::string hlslCode;
    int id = 0;
    Render r;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();
    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    nevk::MdlHlslCodeGen* codeGen = new MdlHlslCodeGen(mTexManager);

    bool res2 = codeGen->init(*runtime);
    CHECK(res2 != 0);

    std::vector<const mi::neuraylib::ICompiled_material*> materials;
    materials.push_back(compiledMaterial.get()); // 1 material
    mi::base::Handle<const mi::neuraylib::ITarget_code> hlsl = codeGen->translate(materials, hlslCode);
    codeGen->loadTextures(hlsl);
    CHECK(mTexManager->textures.size() == 3);
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);

    nevk::ShaderManager* sm = new nevk::ShaderManager();
    uint32_t pixelShaderId = sm->loadShader("../../shaders/test/test_shader.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);
    CHECK(pixelShaderId != -1);
    CHECK(sm->loadShader("../../shaders/test/test_shader.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex) != -1);
    hlslMaterial << hlslCode;
}
