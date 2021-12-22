#include "materialmanager.h"

#include <render/render.h>

#include <doctest.h>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace nevk;
namespace fs = std::filesystem;

TEST_CASE("mdl to hlsl code gen test")
{
    using namespace std;
    const fs::path cwd = fs::current_path();
    cout << cwd.c_str() << endl;

    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    std::string ptFile = cwd.string() + "/shaders/newPT.hlsl";
    std::ofstream outHLSLShaderFile(ptFile.c_str());

    Render r;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();

    MaterialManager* matMngr = new MaterialManager();
    CHECK(matMngr);
    const char* path[2] = { "misc/test_data/mdl", "misc/test_data/mdl/resources" };
    bool res = matMngr->addMdlSearchPath(path, 2);
    CHECK(res);

    MaterialManager::Module* currModule = matMngr->createModule("carbon_composite.mdl");
    CHECK(currModule);
    MaterialManager::MaterialInstance* materialInst1 = matMngr->createMaterialInstance(currModule, "carbon_composite");
    CHECK(materialInst1);
    MaterialManager::CompiledMaterial* materialComp1 = matMngr->compileMaterial(materialInst1);
    CHECK(materialComp1);

    MaterialManager::Module* currModule2 = matMngr->createModule("brushed_antique_copper.mdl");
    CHECK(currModule2);
    MaterialManager::MaterialInstance* materialInst2 = matMngr->createMaterialInstance(currModule2, "brushed_antique_copper");
    CHECK(materialInst2);
    MaterialManager::CompiledMaterial* materialComp2 = matMngr->compileMaterial(materialInst2);
    CHECK(materialComp2);

    std::vector<MaterialManager::CompiledMaterial*> materials;
    materials.push_back(materialComp1);
    materials.push_back(materialComp2);
    CHECK(materials.size() == 2);

    const MaterialManager::TargetCode* code = matMngr->generateTargetCode(materials);
    CHECK(code);
    const char* hlsl = matMngr->getShaderCode(code);
    // std::cout << hlsl << std::endl;

    uint32_t size = matMngr->getArgBufferSize(code);
    CHECK(size != 0);
    size = matMngr->getArgBufferSize(code);
    CHECK(size != 0);
    size = matMngr->getResourceInfoSize(code);
    CHECK(size != 0);

    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    uint32_t texSize = matMngr->getTextureCount(code);
    CHECK(texSize == 8);
    for (uint32_t i = 1; i < texSize; ++i)
    {
        const float* data = matMngr->getTextureData(code, i);
        uint32_t width = matMngr->getTextureWidth(code, i);
        uint32_t height = matMngr->getTextureHeight(code, i);
        const char* type = matMngr->getTextureType(code, i);
        std::string name = matMngr->getTextureName(code, i);
        mTexManager->loadTextureMdl(data, width, height, type, name);
    }

    CHECK(mTexManager->textures.size() == 7);
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);
}

TEST_CASE("mtlx to mdl code gen test")
{
    using namespace std;
    const fs::path cwd = fs::current_path();
    cout << cwd.c_str() << endl;

    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    std::string ptFile = cwd.string() + "/shaders/newPT.hlsl";
    std::ofstream outHLSLShaderFile(ptFile.c_str());

    Render r;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();

    MaterialManager* matMngr = new MaterialManager();
    CHECK(matMngr);
    const char* path[2] = { "/Users/jswark/school/USD_Build/mdl/", "misc/test_data/mtlx" }; // todo: configure paths
    bool res = matMngr->addMdlSearchPath(path, 2);
    CHECK(res);

    std::string file = "misc/test_data/mtlx/standard_surface_wood_tiled.mtlx";
    MaterialManager::Module* currModule = matMngr->createMtlxModule(file.c_str());
    CHECK(currModule);
    MaterialManager::MaterialInstance* materialInst1 = matMngr->createMaterialInstance(currModule, "");
    CHECK(materialInst1);
    MaterialManager::CompiledMaterial* materialComp1 = matMngr->compileMaterial(materialInst1);
    CHECK(materialComp1);

    std::vector<MaterialManager::CompiledMaterial*> materials;
    materials.push_back(materialComp1);

    const MaterialManager::TargetCode* code = matMngr->generateTargetCode(materials);
    CHECK(code);
    const char* hlsl = matMngr->getShaderCode(code);
    //std::cout << hlsl << std::endl;

    uint32_t size = matMngr->getArgBufferSize(code);
    CHECK(size != 0);
    size = matMngr->getArgBufferSize(code);
    CHECK(size != 0);
    size = matMngr->getResourceInfoSize(code);
    CHECK(size != 0);

    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    uint32_t texSize = matMngr->getTextureCount(code);
    for (uint32_t i = 1; i < texSize; ++i)
    {
        const float* data = matMngr->getTextureData(code, i);
        uint32_t width = matMngr->getTextureWidth(code, i);
        uint32_t height = matMngr->getTextureHeight(code, i);
        const char* type = matMngr->getTextureType(code, i);
        std::string name = matMngr->getTextureName(code, i);
        if (data != NULL) // todo: for bsdf_text data is NULL in COMPILATION_CLASS. in default class there is no bsdf_tex
        {
            mTexManager->loadTextureMdl(data, width, height, type, name);
        }
    }

    CHECK(mTexManager->textures.size() == 2);
    CHECK(mTexManager->textures[0].texWidth == 512);
    CHECK(mTexManager->textures[0].texHeight == 512);
}
