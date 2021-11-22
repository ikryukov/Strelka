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
    MaterialManager::Material* material = matMngr->createMaterial(currModule, "carbon_composite");
    CHECK(material);
    std::vector<MaterialManager::Material*> materials;
    materials.push_back(material);
    currModule = matMngr->createModule("brushed_antique_copper.mdl");
    CHECK(currModule);
    material = matMngr->createMaterial(currModule, "brushed_antique_copper");
    CHECK(material);
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

    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    uint32_t texSize = matMngr->getTextureCount(code);
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
    const char* path[2] = { "/Users/jswark/school/USD_Build/mdl/", "misc/test_data/mtlx" };
    bool res = matMngr->addMdlSearchPath(path, 2);
    CHECK(res);

    std::string file = "misc/test_data/mtlx/standard_surface_greysphere_calibration.mtlx";
    MaterialManager::Module* currModule = matMngr->createMtlxModule(file.c_str());
    CHECK(currModule);
    MaterialManager::Material* material = matMngr->createMaterial(currModule, "");
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

    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    uint32_t texSize = matMngr->getTextureCount(code);
    for (uint32_t i = 1; i < texSize; ++i)
    {
        const float* data = matMngr->getTextureData(code, i);
        uint32_t width = matMngr->getTextureWidth(code, i);
        uint32_t height = matMngr->getTextureHeight(code, i);
        const char* type = matMngr->getTextureType(code, i);
        std::string name = matMngr->getTextureName(code, i);
        mTexManager->loadTextureMdl(data, width, height, type, name);
    }

    CHECK(mTexManager->textures.size() == 1);
    CHECK(mTexManager->textures[0].texWidth == 1024);
    CHECK(mTexManager->textures[0].texHeight == 1024);
}