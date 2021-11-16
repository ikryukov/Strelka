#pragma once

#include "ShaderManager.h"
#include "mdlHlslCodeGen.h"
#include "mdlMaterialCompiler.h"
#include "mdlNeurayLoader.h"
#include "mdlRuntime.h"
#include "mtlxMdlCodeGen.h"

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <mi/mdl_sdk.h>

#include <iostream>
#include <filesystem>
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

        matCompiler = new nevk::MdlMaterialCompiler(*runtime);
        matCompiler->compileMaterial(mdlSrc, ident, compiledMaterial); // todo: fix create module if mtlx & make module name
        materials.push_back(compiledMaterial.get());

        codeGen = new MdlHlslCodeGen(mTexManager);
        codeGen->init(*runtime);
        targetHlsl = codeGen->translate(materials, hlslCode, mInternalsInfo);
        codeGen->loadTextures(targetHlsl);

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

    TextureManager* mTexManager = nullptr;

    std::vector<std::string> mdlPaths;
    std::string resourcePath;

    void configurePaths(bool isMtlx)
    {
        using namespace std;
        const fs::path cwd = fs::current_path();

        if (isMtlx)
        {
            mdlPaths.emplace_back("/Users/jswark/school/USD_Build/mdl/");
            resourcePath = "/Users/jswark/school/USD_Build/resources/Materials/Examples/StandardSurface"; // for mtlx
        }
        else
        {
            std::string pathToMdlLib = cwd.string() + "/misc/test_data/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // if mdl -> hlsl
            std::string pathToCoreLib = cwd.string() + "/misc/test_data/mdl-sdk/examples/mdl";
            mdlPaths.push_back(pathToMdlLib);
            mdlPaths.push_back(pathToCoreLib);
            resourcePath = cwd.string() + "/misc/test_data/mdl-sdk/examples/mdl/nvidia/sdk_examples/resources"; // path to the textures
            mdlSrc = cwd.string() + "/misc/test_data/mdl-sdk/examples/mdl/nvidia/sdk_examples/"; // path to the material
        }
#ifdef MI_PLATFORM_WINDOWS
        pathso = cwd.string() + "/misc/test_data/mdl-sdk/nt-x86-64/lib";
        imagePluginPath =  cwd.string() + "/misc/test_data/mdl-sdk/nt-x86-64/lib/nv_freeimage.so";
#else
        pathso = cwd.string() + "/misc/test_data/mdl-sdk/macosx-x86-64/lib";
        imagePluginPath = cwd.string() + "/misc/test_data/mdl-sdk/macosx-x86-64/lib/nv_freeimage.so";
#endif
    }

    std::string imagePluginPath;
    std::string pathso;
    // mtlx -> hlsl
    std::string mtlxMaterialPath = "/Users/jswark/school/USD_Build/resources/Materials/Examples/StandardSurface/standard_surface_plastic.mtlx"; //brass_tiled.mtlx"; -- w/ images
    std::string mtlxLibPath = "/Users/jswark/school/USD_Build/libraries";
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

    std::vector<uint8_t> loadArgBlocks()
    {
       /* for (int i = 0; i < materials.size(); i++)
        {
            mi::Size argLayoutIndex = mInternalsInfo[i].argument_block_index;
            if (argLayoutIndex != static_cast<mi::Size>(-1) &&
                argLayoutIndex < targetHlsl->get_argument_layout_count())
            {
                // argument block for class compilation parameter data
                mi::base::Handle<const mi::neuraylib::ITarget_argument_block> arg_block;
                {
                    // get the layout
                    mi::base::Handle<const mi::neuraylib::ITarget_value_layout> arg_layout(
                        targetHlsl->get_argument_block_layout(argLayoutIndex));

                    // for the first instances of the materials, the argument block already exists
                    // for further blocks new ones have to be created. To avoid special treatment,
                    // an new block is created for every material
                    mi::base::Handle<mi::neuraylib::ITarget_resource_callback> callback(
                        targetHlsl->create_resource_callback(this));

                    arg_block = targetHlsl->create_argument_block(
                        argLayoutIndex,
                        materials[i],
                        callback.get());

                    if (!arg_block)
                    {
                        std::cerr << ("Failed to create material argument block: ") << std::endl;
                        return false;
                    }
                }
                // create a buffer to provide those parameters to the shader
                size_t buffer_size = round_to_power_of_two(arg_block->get_size(), 4);
                argBlockData = std::vector<uint8_t>(buffer_size, 0);
                memcpy(argBlockData.data(), arg_block->get_data(), arg_block->get_size());
            }
        }*/

        argBlockData.resize(4);
        return argBlockData;
    };

    std::vector<uint8_t> loadROData(){
        size_t ro_data_seg_index = 0; // assuming one material per target code only
        if (targetHlsl->get_ro_data_segment_count() > 0)
        {
            const char* data = targetHlsl->get_ro_data_segment_data(ro_data_seg_index);
            size_t dataSize = targetHlsl->get_ro_data_segment_size(ro_data_seg_index);
            const char* name = targetHlsl->get_ro_data_segment_name(ro_data_seg_index);

            std::cerr << name << std::endl;

            roData.resize(dataSize);
            if (dataSize != 0)
            {
                memcpy(roData.data(), data, dataSize);
            }
        }

        if (roData.empty())
        {
            roData.resize(4);
        }

        return roData;
    };

    std::vector<uint8_t> argBlockData;
    std::vector<uint8_t> roData;


    bool createSampler()
    {
        // sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        //VkResult res = vkCreateSampler(mSharedCtx.mDevice, &samplerInfo, nullptr, &mMaterialSampler);
        /*if (res != VK_SUCCESS)
        {
            // error
            assert(0);
        }*/

        return true;
    }
};
} // namespace nevk
