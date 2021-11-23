#include "materialmanager.h"

#include "ShaderManager.h"
#include "materials.h"
#include "mdlHlslCodeGen.h"
#include "mdlLogger.h"
#include "mdlMaterialCompiler.h"
#include "mdlNeurayLoader.h"
#include "mdlRuntime.h"
#include "mtlxMdlCodeGen.h"

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>
#include <MaterialXCore/Material.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>
#include <mi/mdl_sdk.h>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <unordered_map>

namespace fs = std::filesystem;


namespace nevk
{
struct MaterialManager::Module
{
    std::string moduleName;
    std::string identifier;
};

struct MaterialManager::Material
{
    mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
};

struct MaterialManager::TargetCode
{
    mi::base::Handle<const mi::neuraylib::ITarget_code> targetCode;
    std::string targetHlsl;
    std::vector<Mdl_resource_info> resourceInfo;
    std::vector<nevk::MdlHlslCodeGen::InternalMaterialInfo> internalsInfo;
    std::vector<uint8_t> argBlockData;
    std::vector<uint8_t> roData;
};

class MaterialManager::Context
{
public:
    Context()
    {
        configurePaths(true);
    };

    ~Context(){};

    bool addMdlSearchPath(const char* paths[], uint32_t numPaths)
    { // resource path + mdl path
        mRuntime = new nevk::MdlRuntime();
        if (!mRuntime->init(paths, numPaths, mPathso.c_str(), mImagePluginPath.c_str()))
        {
            return false;
        }

        mMtlxCodeGen = new nevk::MtlxMdlCodeGen(mtlxLibPath.c_str());

        mTransaction = mi::base::Handle<mi::neuraylib::ITransaction>(mRuntime->getTransaction());
        mNeuray = mRuntime->getNeuray();
        return true;
    }

    Module* createMtlxModule(const char* file)
    {
        Module* module = new Module;

        mMtlxCodeGen->translate(file, mMdlSrc, module->identifier);

        std::string mdlFile = "misc/test_data/mtlx/" + module->identifier + ".mdl";
        std::ofstream material(mdlFile.c_str());
        material << mMdlSrc;
        material.close();

        mMatCompiler = new nevk::MdlMaterialCompiler(*mRuntime);
        if (!mMatCompiler->createModule(module->identifier, module->moduleName))
        {
            return nullptr;
        }

        return module;
    };

    Module* createModule(const char* file)
    {
        Module* module = new Module;

        const fs::path materialFile = file;
        module->identifier = materialFile.stem().string();

        mMatCompiler = new nevk::MdlMaterialCompiler(*mRuntime);
        if (!mMatCompiler->createModule(module->identifier, module->moduleName))
        {
            return nullptr;
        }

        return module;
    };

    void destroyModule(Module* module)
    {
        assert(module);
        delete module;
    };

    Material* createMaterial(const Module* module, const char* materialName)
    {
        assert(module);
        assert(materialName);

        Material* material = new Material;
        if (!mMatCompiler->createCompiledMaterial(module->moduleName.c_str(), module->identifier.c_str(), material->compiledMaterial)) // need to be checked for dif material names in module. it should be different module names for dif. materials in case of mdl -> hlsl
        {
            return nullptr;
        }

        return material;
    };

    void destroyMaterial(Material* materials)
    {
        assert(materials);
        delete materials;
    }

    const TargetCode* generateTargetCode(const std::vector<Material*>& materials)
    {
        TargetCode* targetCode = new TargetCode;

        mCodeGen = new MdlHlslCodeGen();
        mCodeGen->init(*mRuntime);

        std::vector<const mi::neuraylib::ICompiled_material*> compiledMaterials;
        for (auto& material : materials)
        {
            compiledMaterials.push_back(material->compiledMaterial.get());
        }

        targetCode->targetCode = mCodeGen->translate(compiledMaterials, targetCode->targetHlsl, targetCode->internalsInfo);
        targetCode->argBlockData = loadArgBlocks(targetCode);
        targetCode->roData = loadROData(targetCode);

        targetCode->resourceInfo.resize(targetCode->targetCode->get_texture_count());
        for (uint32_t i = 0; i < targetCode->targetCode->get_texture_count(); ++i)
        {
            targetCode->resourceInfo[i].gpu_resource_array_start = i;
        }

        return targetCode;
    };

    const char* getShaderCode(const TargetCode* targetCode)
    {
        return targetCode->targetHlsl.c_str();
    }

    uint32_t getReadOnlyBlockSize(const TargetCode* shaderCode)
    {
        return shaderCode->argBlockData.size();
    }

    const uint8_t* getReadOnlyBlockData(const TargetCode* targetCode)
    {
        return targetCode->roData.data();
    }

    uint32_t getArgBufferSize(const TargetCode* shaderCode)
    {
        return shaderCode->argBlockData.size();
    }
    const uint8_t* getArgBufferData(const TargetCode* targetCode)
    {
        return targetCode->argBlockData.data();
    }

    uint32_t getResourceInfoSize(const TargetCode* targetCode)
    {
        return targetCode->internalsInfo.size() * sizeof(MdlHlslCodeGen::InternalMaterialInfo);
    }

    const uint8_t* getResourceInfoData(const TargetCode* targetCode)
    {
        return reinterpret_cast<const uint8_t*>(targetCode->resourceInfo.data());
    }

    uint32_t getTextureCount(const TargetCode* targetCode)
    {
        return targetCode->targetCode->get_texture_count();
    }

    const char* getTextureName(const TargetCode* targetCode, uint32_t index)
    {
        assert(index); // index == 0 is invalid
        return targetCode->targetCode->get_texture(index);
    }

    const float* getTextureData(const TargetCode* targetCode, uint32_t index)
    {
        assert(index); // index == 0 is invalid
        assert(mTransaction);

        auto cachedCanvas = m_indexToCanvas.find(index);

        if (cachedCanvas == m_indexToCanvas.end())
        {

            mi::base::Handle<mi::neuraylib::IImage_api> image_api(mNeuray->get_api_component<mi::neuraylib::IImage_api>());
            mi::base::Handle<const mi::neuraylib::ITexture> texture(mTransaction->access<mi::neuraylib::ITexture>(targetCode->targetCode->get_texture(index)));
            mi::base::Handle<const mi::neuraylib::IImage> image(mTransaction->access<mi::neuraylib::IImage>(texture->get_image()));
            mi::base::Handle<const mi::neuraylib::ICanvas> canvas(image->get_canvas());
            char const* image_type = image->get_type();

            if (canvas->get_tiles_size_x() != 1 || canvas->get_tiles_size_y() != 1)
            {
                mLogger->message(mi::base::MESSAGE_SEVERITY_ERROR, "The example does not support tiled images!");
                return nullptr;
            }

            if (texture->get_effective_gamma() != 1.0f)
            {
                // Copy/convert to float4 canvas and adjust gamma from "effective gamma" to 1.
                mi::base::Handle<mi::neuraylib::ICanvas> gamma_canvas(image_api->convert(canvas.get(), "Color"));
                gamma_canvas->set_gamma(texture->get_effective_gamma());
                image_api->adjust_gamma(gamma_canvas.get(), 1.0f);
                canvas = gamma_canvas;
            }
            else if (strcmp(image_type, "Color") != 0 && strcmp(image_type, "Float32<4>") != 0)
            {
                // Convert to expected format
                canvas = image_api->convert(canvas.get(), "Color");
            }
            m_indexToCanvas[index] = canvas;
            cachedCanvas = m_indexToCanvas.find(index);
        }
        
        mi::Float32 const* data = nullptr;
        mi::neuraylib::ITarget_code::Texture_shape texture_shape = targetCode->targetCode->get_texture_shape(index);
        if (texture_shape == mi::neuraylib::ITarget_code::Texture_shape_2d)
        {
            mi::base::Handle<const mi::neuraylib::ITile> tile(cachedCanvas->second->get_tile());
            data = static_cast<mi::Float32 const*>(tile->get_data());
        }

        return data;
    }

    const char* getTextureType(const TargetCode* targetCode, uint32_t index)
    {
        assert(index); // index == 0 is invalid
        mi::base::Handle<const mi::neuraylib::ITexture> texture(
            mTransaction->access<mi::neuraylib::ITexture>(targetCode->targetCode->get_texture(index)));
        mi::base::Handle<const mi::neuraylib::IImage> image(
            mTransaction->access<mi::neuraylib::IImage>(texture->get_image()));
        char const* imageType = image->get_type();

        return imageType;
    }

    uint32_t getTextureWidth(const TargetCode* targetCode, uint32_t index)
    {
        assert(index); // index == 0 is invalid
        mi::base::Handle<const mi::neuraylib::ITexture> texture(
            mTransaction->access<mi::neuraylib::ITexture>(targetCode->targetCode->get_texture(index)));
        mi::base::Handle<const mi::neuraylib::IImage> image(
            mTransaction->access<mi::neuraylib::IImage>(texture->get_image()));
        mi::base::Handle<const mi::neuraylib::ICanvas> canvas(image->get_canvas());
        mi::Uint32 texWidth = canvas->get_resolution_x();

        return texWidth;
    }

    uint32_t getTextureHeight(const TargetCode* targetCode, uint32_t index)
    {
        assert(index); // index == 0 is invalid
        mi::base::Handle<const mi::neuraylib::ITexture> texture(
            mTransaction->access<mi::neuraylib::ITexture>(targetCode->targetCode->get_texture(index)));
        mi::base::Handle<const mi::neuraylib::IImage> image(
            mTransaction->access<mi::neuraylib::IImage>(texture->get_image()));
        mi::base::Handle<const mi::neuraylib::ICanvas> canvas(image->get_canvas());
        mi::Uint32 texHeight = canvas->get_resolution_y();

        return texHeight;
    }

    uint32_t getTextureBytesPerTexel(const TargetCode* targetCode, uint32_t index)
    {
        assert(index); // index == 0 is invalid
        mi::base::Handle<mi::neuraylib::IImage_api> image_api(mNeuray->get_api_component<mi::neuraylib::IImage_api>());

        mi::base::Handle<const mi::neuraylib::ITexture> texture(
            mTransaction->access<mi::neuraylib::ITexture>(targetCode->targetCode->get_texture(index)));
        mi::base::Handle<const mi::neuraylib::IImage> image(
            mTransaction->access<mi::neuraylib::IImage>(texture->get_image()));
        char const* imageType = image->get_type();

        int cpp = image_api->get_components_per_pixel(imageType);
        int bpc = image_api->get_bytes_per_component(imageType);
        int bpp = cpp * bpc;

        return bpp;
    }

private:
    std::string mImagePluginPath;
    std::string mPathso;
    std::string mMdlSrc;
    std::string hlslCode;
    std::string mtlxLibPath;

    std::unordered_map<uint32_t, mi::base::Handle<const mi::neuraylib::ICanvas>> m_indexToCanvas;

    void configurePaths(bool isMtlx)
    {
        using namespace std;
        const fs::path cwd = fs::current_path();
        mtlxLibPath = "/Users/jswark/school/USD_Build/libraries";
        mMdlSrc = cwd.string() + "/misc/test_data/mdl/"; // path to the material

//        if (!isMtlx)
        {
#ifdef MI_PLATFORM_WINDOWS
            mPathso = cwd.string();
            mImagePluginPath = cwd.string() + "/nv_freeimage.dll";
#else
            mPathso = cwd.string();
            mImagePluginPath = cwd.string() + "/nv_freeimage.so";
#endif
        }
        //else
        //{
        //    mPathso = "/Users/jswark/Desktop/school/NeVKf/external/mdl-sdk/macosx-x86-64/lib";
        //    mImagePluginPath = cwd.string() + "/nv_freeimage.so";
        //}
    }

    nevk::MdlHlslCodeGen* mCodeGen = nullptr;
    nevk::MdlMaterialCompiler* mMatCompiler = nullptr;
    nevk::MdlRuntime* mRuntime = nullptr;
    nevk::MtlxMdlCodeGen* mMtlxCodeGen = nullptr;

    mi::base::Handle<mi::neuraylib::ITransaction> mTransaction;
    mi::base::Handle<MdlLogger> mLogger;
    mi::base::Handle<mi::neuraylib::INeuray> mNeuray;

    std::vector<uint8_t> loadArgBlocks(const TargetCode* targetCode);
    std::vector<uint8_t> loadROData(const TargetCode* targetCode);
};

MaterialManager::MaterialManager()
{
    mContext = std::make_unique<Context>();
}

MaterialManager::~MaterialManager()
{
    mContext.reset(nullptr);
};

bool MaterialManager::addMdlSearchPath(const char** paths, uint32_t numPaths)
{
    return mContext->addMdlSearchPath(paths, numPaths);
}
MaterialManager::Module* MaterialManager::createModule(const char* file)
{
    return mContext->createModule(file);
}
MaterialManager::Module* MaterialManager::createMtlxModule(const char* file)
{
    return mContext->createMtlxModule(file);
}
void MaterialManager::destroyModule(MaterialManager::Module* module)
{
    return mContext->destroyModule(module);
}
MaterialManager::Material* MaterialManager::createMaterial(const MaterialManager::Module* module, const char* materialName)
{
    return mContext->createMaterial(module, materialName);
}
void MaterialManager::destroyMaterial(Material* materials)
{
    return mContext->destroyMaterial(materials);
}
const MaterialManager::TargetCode* MaterialManager::generateTargetCode(const std::vector<MaterialManager::Material*>& materials)
{
    return mContext->generateTargetCode(materials);
}
const char* MaterialManager::getShaderCode(const TargetCode* targetCode) // get shader code
{
    return mContext->getShaderCode(targetCode);
}

uint32_t MaterialManager::getReadOnlyBlockSize(const TargetCode* shaderCode)
{
    return mContext->getReadOnlyBlockSize(shaderCode);
}

const uint8_t* MaterialManager::getReadOnlyBlockData(const TargetCode* targetCode)
{
    return mContext->getReadOnlyBlockData(targetCode);
}

uint32_t MaterialManager::getArgBufferSize(const TargetCode* shaderCode)
{
    return mContext->getArgBufferSize(shaderCode);
}

const uint8_t* MaterialManager::getArgBufferData(const TargetCode* targetCode)
{
    return mContext->getArgBufferData(targetCode);
}

uint32_t MaterialManager::getResourceInfoSize(const TargetCode* targetCode)
{
    return mContext->getResourceInfoSize(targetCode);
}

const uint8_t* MaterialManager::getResourceInfoData(const TargetCode* targetCode)
{
    return mContext->getResourceInfoData(targetCode);
}

uint32_t MaterialManager::getTextureCount(const TargetCode* targetCode)
{
    return mContext->getTextureCount(targetCode);
}

const char* MaterialManager::getTextureName(const TargetCode* targetCode, uint32_t index)
{
    return mContext->getTextureName(targetCode, index);
}

const float* MaterialManager::getTextureData(const TargetCode* targetCode, uint32_t index)
{
    return mContext->getTextureData(targetCode, index);
}

const char* MaterialManager::getTextureType(const TargetCode* targetCode, uint32_t index)
{
    return mContext->getTextureType(targetCode, index);
}

uint32_t MaterialManager::getTextureWidth(const TargetCode* targetCode, uint32_t index)
{
    return mContext->getTextureWidth(targetCode, index);
}

uint32_t MaterialManager::getTextureHeight(const TargetCode* targetCode, uint32_t index)
{
    return mContext->getTextureHeight(targetCode, index);
}

uint32_t MaterialManager::getTextureBytesPerTexel(const TargetCode* targetCode, uint32_t index)
{
    return mContext->getTextureBytesPerTexel(targetCode, index);
}

std::vector<uint8_t> MaterialManager::Context::loadArgBlocks(const TargetCode* targetCode)
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

    std::vector<uint8_t> argBlockData;
    argBlockData.resize(4);
    return argBlockData;
};

std::vector<uint8_t> MaterialManager::Context::loadROData(const TargetCode* targetCode)
{
    std::vector<uint8_t> roData;

    size_t ro_data_seg_index = 0; // assuming one material per target code only
    if (targetCode->targetCode->get_ro_data_segment_count() > 0)
    {
        const char* data = targetCode->targetCode->get_ro_data_segment_data(ro_data_seg_index);
        size_t dataSize = targetCode->targetCode->get_ro_data_segment_size(ro_data_seg_index);
        const char* name = targetCode->targetCode->get_ro_data_segment_name(ro_data_seg_index);

        // std::cerr << name << std::endl;

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
}
} // namespace nevk
