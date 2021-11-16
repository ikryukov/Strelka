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
#include <iostream>
#include <filesystem>

using namespace nevk;
namespace fs = std::filesystem;

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
    using namespace std;
    const fs::path cwd = fs::current_path();
    cout << cwd.c_str() << endl;
    
    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    std::string ptFile = cwd.string() + "/shaders/newPT.hlsl";
    std::ofstream hlslMaterial(ptFile.c_str());

    Render r;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();
    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());

    MaterialManager* matMngr = new MaterialManager(mTexManager);

    CHECK(mTexManager->textures.size() == 3);
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);

    std::ifstream pt(cwd.string() + "/shaders/pathtracer.hlsl");
    std::stringstream ptcode;
    ptcode << pt.rdbuf();

    nevk::ShaderManager* sm = new nevk::ShaderManager();
    std::string newPTfile = matMngr->hlslCode + "\n" + ptcode.str();
    uint32_t shaderIdString = sm->loadShaderFromString(newPTfile.c_str(), "computeMain", nevk::ShaderManager::Stage::eCompute);
    CHECK(shaderIdString != -1);

    hlslMaterial << newPTfile;
    uint32_t shaderIdFile = sm->loadShader(ptFile.c_str(), "computeMain", nevk::ShaderManager::Stage::eCompute);
    CHECK(shaderIdFile != -1);
}
