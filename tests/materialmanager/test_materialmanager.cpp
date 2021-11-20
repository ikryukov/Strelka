#include "materialmanager.h"

#include <render/render.h>

#include <doctest.h>
#include <filesystem>
#include <fstream>
#include <iostream>

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
    std::ofstream outHLSLShaderFile(ptFile.c_str());

    MaterialManager* matMngr = new MaterialManager();
    CHECK(matMngr);
    const char* path[2] = { "misc/test_data/mdl", "misc/test_data/mdl/resources" };
    bool res = matMngr->addMdlSearchPath(path, 2);
    CHECK(res);
    MaterialManager::Module* currModule = matMngr->createModule("carbon_composite.mdl");
    CHECK(currModule);
    MaterialManager::Material* material = matMngr->createMaterial(currModule, "carbon_composite");
    CHECK(material);
    std::vector<MaterialManager::Material*> materials;
    materials.push_back(material);
    const MaterialManager::TargetCode* code = matMngr->generateTargetCode(materials);
    CHECK(code);
    const char* hlsl = matMngr->getShaderCode(code);
    std::cout << hlsl << std::endl;

    uint32_t size = matMngr->getArgBufferSize(code);
    CHECK(size != 0);
    size = matMngr->getArgBufferSize(code);
    CHECK(size != 0);
    size = matMngr->getResourceInfoSize(code);
    CHECK(size != 0);

    Render r;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();

    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    uint32_t texSize = matMngr->getTextureCount(code);
    for (uint32_t i = 0; i < texSize; ++i)
    {
        const float* data = matMngr->getTextureData(code, i);
        uint32_t width = matMngr->getTextureWidth(code, i);
        uint32_t height = matMngr->getTextureHeight(code, i);
        const char* type = matMngr->getTextureType(code, i);
        res = mTexManager->loadTextureMdl(data, width, height, type, to_string(i));
        CHECK(res);
    }

    CHECK(mTexManager->textures.size() == 3); // not sure
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);
}
