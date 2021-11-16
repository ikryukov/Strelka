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

        mRuntime = new nevk::MdlRuntime();
        mRuntime->init(mResourcePath.c_str(), mPathso.c_str(), mMdlPaths, mImagePluginPath.c_str());
        mNeuray = mRuntime->getNeuray();

        mMatCompiler = new nevk::MdlMaterialCompiler(*mRuntime);
        mMatCompiler->compileMaterial(mMdlSrc, mIdentifier, mCompiledMaterial); // todo: fix create module if mtlx & make module name
        mMaterials.push_back(mCompiledMaterial.get());

        mCodeGen = new MdlHlslCodeGen(mTexManager);
        mCodeGen->init(*mRuntime);
        mTargetHlsl = mCodeGen->translate(mMaterials, hlslCode, mInternalsInfo);

        mLogger = mi::base::Handle<MdlLogger>(mRuntime->getLogger());
        mTransaction = mi::base::Handle<mi::neuraylib::ITransaction>(mRuntime->getTransaction());

        loadTextures(mTargetHlsl);

        loadArgBlocks();
        loadROData();

        createSampler();
    };

    std::string hlslCode;

    ~MaterialManager(){};

private:
    std::vector<nevk::MdlHlslCodeGen::InternalMaterialInfo> mInternalsInfo;

    nevk::MdlHlslCodeGen* mCodeGen = nullptr;
    nevk::MdlMaterialCompiler* mMatCompiler = nullptr;
    nevk::MdlRuntime* mRuntime = nullptr;
    nevk::MtlxMdlCodeGen* mMtlxCodeGen = nullptr;

    mi::base::Handle<mi::neuraylib::ITransaction> mTransaction;
    mi::base::Handle<MdlLogger> mLogger;
    mi::base::Handle<mi::neuraylib::INeuray> mNeuray;

    std::vector<Mdl_resource_info> mInfo;

    TextureManager* mTexManager = nullptr;

    std::vector<std::string> mMdlPaths;
    std::string mResourcePath;

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
            mMdlPaths.push_back(pathToMdlLib);
            mMdlPaths.push_back(pathToCoreLib);
            mResourcePath = cwd.string() + "/misc/test_data/mdl/resources"; // path to the textures
            mMdlSrc = cwd.string() + "/misc/test_data/mdl/"; // path to the material
        }
#ifdef MI_PLATFORM_WINDOWS
        mPathso = cwd.string();
        mImagePluginPath = cwd.string() + "/nv_freeimage.dll";
#else
        mPathso = cwd.string();
        mImagePluginPath = cwd.string() + "/nv_freeimage.so";
#endif
    }

    std::string mImagePluginPath;
    std::string mPathso;
    // mtlx -> hlsl
    // std::string mtlxMaterialPath = "/Users/jswark/school/USD_Build/resources/Materials/Examples/StandardSurface/standard_surface_plastic.mtlx"; //brass_tiled.mtlx"; -- w/ images
    // std::string mtlxLibPath = "/Users/jswark/school/USD_Build/libraries";
    // mdl -> hlsl
    std::string mMdlSrc;
    std::string mIdentifier = "carbon_composite"; //todo: the identifier depends on a material file // empty for mtlx mode

    mi::base::Handle<mi::neuraylib::ICompiled_material> mCompiledMaterial;
    std::vector<const mi::neuraylib::ICompiled_material*> mMaterials;
    mi::base::Handle<const mi::neuraylib::ITarget_code> mTargetHlsl;

    VkSampler mMaterialSampler;

    inline size_t round_to_power_of_two(size_t value, size_t power_of_two_factor)
    {
        return (value + (power_of_two_factor - 1)) & ~(power_of_two_factor - 1);
    }

    std::vector<uint8_t> mArgBlockData;
    std::vector<uint8_t> mRoData;
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
