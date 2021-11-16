#pragma once

#include "ShaderManager.h"
#include "mdlHlslCodeGen.h"
#include "mdlLogger.h"
#include "mdlMaterialCompiler.h"
#include "mdlNeurayLoader.h"
#include "mdlRuntime.h"
#include "mtlxMdlCodeGen.h"

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <mi/mdl_sdk.h>

#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

namespace nevk
{

class MaterialManager
{
public:
    MaterialManager(TextureManager* texManager)
        : mTexManager(texManager)
    {
        // if mdl -> hlsl
        configurePaths(false);

        // if mtlx -> mdl
        /* configurePaths(true);
        mtlxCodeGen = new nevk::MtlxMdlCodeGen(mtlxLibPath.c_str());
        mtlxCodeGen->translate(mtlxMaterialPath.c_str(), mdlSrc, ident); */
        //

        runtime = new nevk::MdlRuntime();
        runtime->init(resourcePath.c_str(), pathso.c_str(), mdlPaths, imagePluginPath.c_str());
        neuray = runtime->getNeuray();

        matCompiler = new nevk::MdlMaterialCompiler(*runtime);
        matCompiler->compileMaterial(mdlSrc, ident, compiledMaterial); // todo: fix create module if mtlx & make module name
        materials.push_back(compiledMaterial.get());

        codeGen = new MdlHlslCodeGen(mTexManager);
        codeGen->init(*runtime);
        targetHlsl = codeGen->translate(materials, hlslCode, mInternalsInfo);

        m_logger = mi::base::Handle<MdlLogger>(runtime->getLogger());
        m_transaction = mi::base::Handle<mi::neuraylib::ITransaction>(runtime->getTransaction());

        loadTextures(targetHlsl);

        loadArgBlocks();
        loadROData();

        createSampler();
    };

    std::string hlslCode;

    ~MaterialManager(){};

private:
    std::vector<nevk::MdlHlslCodeGen::InternalMaterialInfo> mInternalsInfo;

    nevk::MdlHlslCodeGen* codeGen = nullptr;
    nevk::MdlMaterialCompiler* matCompiler = nullptr;
    nevk::MdlRuntime* runtime = nullptr;
    nevk::MtlxMdlCodeGen* mtlxCodeGen = nullptr;

    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    mi::base::Handle<MdlLogger> m_logger;
    mi::base::Handle<mi::neuraylib::INeuray> neuray;

    std::vector<Mdl_resource_info> mInfo;

    TextureManager* mTexManager = nullptr;

    std::vector<std::string> mdlPaths;
    std::string resourcePath;

    void configurePaths(bool isMtlx)
    {
        using namespace std;
        const fs::path cwd = fs::current_path();

        if (isMtlx)
        {
            // mdlPaths.emplace_back("/Users/jswark/school/USD_Build/mdl/");
            // resourcePath = "/Users/jswark/school/USD_Build/resources/Materials/Examples/StandardSurface"; // for mtlx
        }
        else
        {
            std::string pathToMdlLib = cwd.string() + "/misc/test_data/mdl/"; // if mdl -> hlsl
            std::string pathToCoreLib = cwd.string() + "/misc/test_data/mdl";
            mdlPaths.push_back(pathToMdlLib);
            mdlPaths.push_back(pathToCoreLib);
            resourcePath = cwd.string() + "/misc/test_data/mdl/resources"; // path to the textures
            mdlSrc = cwd.string() + "/misc/test_data/mdl/"; // path to the material
        }
#ifdef MI_PLATFORM_WINDOWS
        pathso = cwd.string();
        imagePluginPath = cwd.string() + "/nv_freeimage.dll";
#else
        pathso = cwd.string();
        imagePluginPath = cwd.string() + "/nv_freeimage.so";
#endif
    }

    std::string imagePluginPath;
    std::string pathso;
    // mtlx -> hlsl
    // std::string mtlxMaterialPath = "/Users/jswark/school/USD_Build/resources/Materials/Examples/StandardSurface/standard_surface_plastic.mtlx"; //brass_tiled.mtlx"; -- w/ images
    // std::string mtlxLibPath = "/Users/jswark/school/USD_Build/libraries";
    // mdl -> hlsl
    std::string mdlSrc;
    std::string ident = "carbon_composite"; //todo: the identifier depends on a material file // empty for mtlx mode

    mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
    std::vector<const mi::neuraylib::ICompiled_material*> materials;
    mi::base::Handle<const mi::neuraylib::ITarget_code> targetHlsl;

    VkSampler mMaterialSampler;

    inline size_t round_to_power_of_two(size_t value, size_t power_of_two_factor)
    {
        return (value + (power_of_two_factor - 1)) & ~(power_of_two_factor - 1);
    }

    std::vector<uint8_t> argBlockData;
    std::vector<uint8_t> roData;
    std::vector<uint8_t> loadArgBlocks();
    std::vector<uint8_t> loadROData();

    bool loadTextures(mi::base::Handle<const mi::neuraylib::ITarget_code>& targetCode);
    bool prepare_texture(
        const mi::base::Handle<mi::neuraylib::ITransaction>& transaction,
        const mi::base::Handle<mi::neuraylib::IImage_api>& image_api,
        const mi::base::Handle<const mi::neuraylib::ITarget_code>& code,
        mi::Size texture_index);

    bool createSampler();
};
} // namespace nevk
