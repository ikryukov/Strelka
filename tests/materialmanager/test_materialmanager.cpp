#include "materialmanager.h"

//#include <render/render.h>

#include <doctest.h>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace oka;
namespace fs = std::filesystem;

TEST_CASE("mtlx to hlsl code gen test")
{
    static const char* DEFAULT_MTLX_DOC_1 =
        "<?xml version=\"1.0\"?>"
        "<materialx version=\"1.38\" colorspace=\"lin_rec709\">"
        "  <UsdPreviewSurface name=\"SR_Invalid\" type=\"surfaceshader\">"
        "    <input name=\"diffuseColor\" type=\"color3\" value=\"1.0, 0.0, 1.0\" />"
        "    <input name=\"roughness\" type=\"float\" value=\"1.0\" />"
        "  </UsdPreviewSurface>"
        "  <surfacematerial name=\"invalid\" type=\"material\">"
        "    <input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"SR_Invalid\" />"
        "  </surfacematerial>"
        "</materialx>";
    static const char* DEFAULT_MTLX_DOC_2 =
        "<?xml version=\"1.0\"?>"
        "<materialx version=\"1.38\" colorspace=\"lin_rec709\">"
        "  <UsdPreviewSurface name=\"SR_Invalid\" type=\"surfaceshader\">"
        "    <input name=\"diffuseColor\" type=\"color3\" value=\"0.0, 0.0, 1.0\" />"
        "    <input name=\"roughness\" type=\"float\" value=\"1.0\" />"
        "  </UsdPreviewSurface>"
        "  <surfacematerial name=\"invalid\" type=\"material\">"
        "    <input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"SR_Invalid\" />"
        "  </surfacematerial>"
        "</materialx>";

    using namespace std;
    const fs::path cwd = fs::current_path();
    cout << cwd.c_str() << endl;

    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    MaterialManager* matMngr = new MaterialManager();
    CHECK(matMngr);
    
    const char* envUSDPath = std::getenv("USD_PATH");
    if (!envUSDPath)
    {
        printf("Please, set USD_PATH variable\n");
        assert(0);
    }
    const std::string usdMdlLibPath = std::string(envUSDPath) + "/mdl";

    const char* paths[4] = { "./misc/test_data/mtlx", "./misc/test_data/mdl/", "./misc/test_data/mdl/resources/",
                             usdMdlLibPath.c_str() };
    bool res = matMngr->addMdlSearchPath(paths, 4);
    CHECK(res);

    MaterialManager::Module* mdlModule = matMngr->createMtlxModule(DEFAULT_MTLX_DOC_1);
    assert(mdlModule);
    MaterialManager::MaterialInstance* materialInst = matMngr->createMaterialInstance(mdlModule, "");
    assert(materialInst);
    MaterialManager::CompiledMaterial* materialComp = matMngr->compileMaterial(materialInst);
    assert(materialComp);

    MaterialManager::Module* mdlModule2 = matMngr->createMtlxModule(DEFAULT_MTLX_DOC_2);
    assert(mdlModule2);
    MaterialManager::MaterialInstance* materialInst2 = matMngr->createMaterialInstance(mdlModule2, "");
    assert(materialInst2);
    MaterialManager::CompiledMaterial* materialComp2 = matMngr->compileMaterial(materialInst2);
    assert(materialComp2);

    std::vector<MaterialManager::CompiledMaterial*> materials;
    materials.push_back(materialComp);
    materials.push_back(materialComp2);

    const MaterialManager::TargetCode* code = matMngr->generateTargetCode(materials);
    CHECK(code);
    const char* hlsl = matMngr->getShaderCode(code);

    std::string shaderFile = "test_material_output.hlsl";
    std::ofstream out (shaderFile.c_str());
    out << hlsl << std::endl;
    out.close();
}

TEST_CASE("mdl to hlsl code gen test")
{
    using namespace std;
    const fs::path cwd = fs::current_path();
    cout << cwd.c_str() << endl;

    std::string mdlFile = "material.mdl";
    std::ofstream mdlMaterial(mdlFile.c_str());

    std::string ptFile = cwd.string() + "/shaders/newPT.hlsl";
    std::ofstream outHLSLShaderFile(ptFile.c_str());

    // Render r;
    // r.HEIGHT = 600;
    // r.WIDTH = 800;
    // r.initWindow();
    // r.initVulkan();

    MaterialManager* matMngr = new MaterialManager();
    CHECK(matMngr);
    const char* path[2] = { "misc/test_data/mdl", "misc/test_data/mdl/resources" };
    bool res = matMngr->addMdlSearchPath(path, 2);
    CHECK(res);

    MaterialManager::Module* currModule = matMngr->createModule("OmniGlass.mdl");
    CHECK(currModule);
    MaterialManager::MaterialInstance* materialInst1 = matMngr->createMaterialInstance(currModule, "OmniGlass");
    CHECK(materialInst1);
    MaterialManager::CompiledMaterial* materialComp1 = matMngr->compileMaterial(materialInst1);
    CHECK(materialComp1);


    std::vector<MaterialManager::CompiledMaterial*> materials;
    materials.push_back(materialComp1);
    CHECK(materials.size() == 1);

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

    // // nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
    // uint32_t texSize = matMngr->getTextureCount(code);
    // CHECK(texSize == 8);
    // for (uint32_t i = 1; i < texSize; ++i)
    // {
    //     const float* data = matMngr->getTextureData(code, i);
    //     uint32_t width = matMngr->getTextureWidth(code, i);
    //     uint32_t height = matMngr->getTextureHeight(code, i);
    //     const char* type = matMngr->getTextureType(code, i);
    //     std::string name = matMngr->getTextureName(code, i);
    //     // mTexManager->loadTextureMdl(data, width, height, type, name);
    // }

    // CHECK(mTexManager->textures.size() == 7);
    // CHECK(mTexManager->textures[0].texWidth == 512);
    // CHECK(mTexManager->textures[0].texHeight == 512);
}

// TEST_CASE("mtlx to mdl code gen test")
// {
//     using namespace std;
//     const fs::path cwd = fs::current_path();
//     cout << cwd.c_str() << endl;

//     std::string mdlFile = "material.mdl";
//     std::ofstream mdlMaterial(mdlFile.c_str());

//     std::string ptFile = cwd.string() + "/shaders/newPT.hlsl";
//     std::ofstream outHLSLShaderFile(ptFile.c_str());

//     Render r;
//     r.HEIGHT = 600;
//     r.WIDTH = 800;
//     r.initWindow();
//     r.initVulkan();

//     MaterialManager* matMngr = new MaterialManager();
//     CHECK(matMngr);
//     const char* path[2] = { "misc/test_data/mdl/", "misc/test_data/mtlx" }; // todo: configure paths
//     bool res = matMngr->addMdlSearchPath(path, 2);
//     CHECK(res);

//     std::string file = "misc/test_data/mtlx/standard_surface_wood_tiled.mtlx";
//     std::unique_ptr<MaterialManager::Module> currModule = matMngr->createMtlxModule(file.c_str());
//     CHECK(currModule);
//     std::unique_ptr<MaterialManager::MaterialInstance> materialInst1 = matMngr->createMaterialInstance(currModule.get(), "");
//     CHECK(materialInst1);
//     std::unique_ptr<MaterialManager::CompiledMaterial> materialComp1 = matMngr->compileMaterial(materialInst1.get());
//     CHECK(materialComp1);

//     std::vector<std::unique_ptr<MaterialManager::CompiledMaterial>> materials;
//     materials.push_back(std::move(materialComp1));

//     const MaterialManager::TargetCode* code = matMngr->generateTargetCode(materials);
//     CHECK(code);
//     const char* hlsl = matMngr->getShaderCode(code);
//     //std::cout << hlsl << std::endl;

//     uint32_t size = matMngr->getArgBufferSize(code);
//     CHECK(size != 0);
//     size = matMngr->getArgBufferSize(code);
//     CHECK(size != 0);
//     size = matMngr->getResourceInfoSize(code);
//     CHECK(size != 0);

//     nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), r.getResManager());
//     uint32_t texSize = matMngr->getTextureCount(code);
//     for (uint32_t i = 1; i < texSize; ++i)
//     {
//         const float* data = matMngr->getTextureData(code, i);
//         uint32_t width = matMngr->getTextureWidth(code, i);
//         uint32_t height = matMngr->getTextureHeight(code, i);
//         const char* type = matMngr->getTextureType(code, i);
//         std::string name = matMngr->getTextureName(code, i);
//         if (data != NULL) // todo: for bsdf_text data is NULL in COMPILATION_CLASS. in default class there is no bsdf_tex
//         {
//             mTexManager->loadTextureMdl(data, width, height, type, name);
//         }
//     }

//     CHECK(mTexManager->textures.size() == 2);
//     CHECK(mTexManager->textures[0].texWidth == 512);
//     CHECK(mTexManager->textures[0].texHeight == 512);
// }
