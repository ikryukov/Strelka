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
    std::vector<MdlMaterial> mdlMaterials;
    std::vector<uint8_t> argBlockData;
    std::vector<uint8_t> roData;
    std::vector<const mi::neuraylib::ICompiled_material*> compiledMaterials;
};

class Resource_callback
    : public mi::base::Interface_implement<mi::neuraylib::ITarget_resource_callback>
{
public:
    /// Constructor.
    Resource_callback(
        mi::base::Handle<mi::neuraylib::ITransaction>& transaction,
        const mi::base::Handle<const mi::neuraylib::ITarget_code>& target_code)
        : m_transaction(transaction),
          m_target_code(target_code)
    {
    }

    /// Destructor.
    virtual ~Resource_callback() = default;

    /// Returns a resource index for the given resource value usable by the target code
    /// resource handler for the corresponding resource type.
    ///
    /// \param resource  the resource value
    ///
    /// \returns a resource index or 0 if no resource index can be returned
    mi::Uint32 get_resource_index(mi::neuraylib::IValue_resource const* resource) override
    {
        // check whether we already know the resource index
        auto it = m_resource_cache.find(resource);
        if (it != m_resource_cache.end())
            return it->second;

        // handle resources already known by the target code
        mi::Uint32 res_idx = m_target_code->get_known_resource_index(m_transaction.get(), resource);
        if (res_idx != 0)
        {
            // only accept body resources
            switch (resource->get_kind())
            {
            case mi::neuraylib::IValue::VK_TEXTURE:
                if (res_idx < m_target_code->get_body_texture_count())
                    return res_idx;
                break;
            case mi::neuraylib::IValue::VK_LIGHT_PROFILE:
                if (res_idx < m_target_code->get_body_light_profile_count())
                    return res_idx;
                break;
            case mi::neuraylib::IValue::VK_BSDF_MEASUREMENT:
                if (res_idx < m_target_code->get_body_bsdf_measurement_count())
                    return res_idx;
                break;
            default:
                return 0u; // invalid kind
            }
        }

        switch (resource->get_kind())
        {
        case mi::neuraylib::IValue::VK_TEXTURE: {
            mi::base::Handle<mi::neuraylib::IValue_texture const> val_texture(
                resource->get_interface<mi::neuraylib::IValue_texture const>());
            if (!val_texture)
                return 0u; // unknown resource

            mi::base::Handle<const mi::neuraylib::IType_texture> texture_type(
                val_texture->get_type());

            mi::neuraylib::ITarget_code::Texture_shape shape =
                mi::neuraylib::ITarget_code::Texture_shape(texture_type->get_shape());

            //m_compile_result.textures.emplace_back(resource->get_value(), shape);
            //res_idx = m_compile_result.textures.size() - 1;
            break;
        }
        case mi::neuraylib::IValue::VK_LIGHT_PROFILE:
            //m_compile_result.light_profiles.emplace_back(resource->get_value());
            //res_idx = m_compile_result.light_profiles.size() - 1;
            break;
        case mi::neuraylib::IValue::VK_BSDF_MEASUREMENT:
            //m_compile_result.bsdf_measurements.emplace_back(resource->get_value());
            //res_idx = m_compile_result.bsdf_measurements.size() - 1;
            break;
        default:
            return 0u; // invalid kind
        }

        m_resource_cache[resource] = res_idx;
        return res_idx;
    }

    /// Returns a string identifier for the given string value usable by the target code.
    ///
    /// The value 0 is always the "not known string".
    ///
    /// \param s  the string value
    mi::Uint32 get_string_index(mi::neuraylib::IValue_string const* s) override
    {
        char const* str_val = s->get_value();
        if (str_val == nullptr)
            return 0u;

        for (mi::Size i = 0, n = m_target_code->get_string_constant_count(); i < n; ++i)
        {
            if (strcmp(m_target_code->get_string_constant(i), str_val) == 0)
                return mi::Uint32(i);
        }

        // string not known by code
        return 0u;
    }

private:
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    mi::base::Handle<const mi::neuraylib::ITarget_code> m_target_code;

    std::unordered_map<mi::neuraylib::IValue_resource const*, mi::Uint32> m_resource_cache;
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

        // use smart pointer
        Material* material = new Material;
        if (!mMatCompiler->createCompiledMaterial(module->moduleName.c_str(), materialName, material->compiledMaterial)) // need to be checked for dif material names in module. it should be different module names for dif. materials in case of mdl -> hlsl
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

        targetCode->mdlMaterials.resize(materials.size());

        for (uint32_t i = 0; i < materials.size(); ++i)
        {
            targetCode->compiledMaterials.push_back(materials[i]->compiledMaterial.get());
            targetCode->mdlMaterials[i].functionId = i; // TODO: ???
        }

        targetCode->targetCode = mCodeGen->translate(targetCode->compiledMaterials, targetCode->targetHlsl, targetCode->internalsInfo);
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
        return shaderCode->roData.size();
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
        return targetCode->resourceInfo.size() * sizeof(Mdl_resource_info);
    }

    const uint8_t* getResourceInfoData(const TargetCode* targetCode)
    {
        return reinterpret_cast<const uint8_t*>(targetCode->resourceInfo.data());
    }

    uint32_t getMdlMaterialSize(const TargetCode* targetCode)
    {
        return targetCode->mdlMaterials.size() * sizeof(MdlMaterial);
    }

    const uint8_t* getMdlMaterialData(const TargetCode* targetCode)
    {
        return reinterpret_cast<const uint8_t*>(targetCode->mdlMaterials.data());
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

bool MaterialManager::addMdlSearchPath(const char* paths[], uint32_t numPaths)
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

uint32_t MaterialManager::getReadOnlyBlockSize(const TargetCode* targetCode)
{
    return mContext->getReadOnlyBlockSize(targetCode);
}

const uint8_t* MaterialManager::getReadOnlyBlockData(const TargetCode* targetCode)
{
    return mContext->getReadOnlyBlockData(targetCode);
}

uint32_t MaterialManager::getArgBufferSize(const TargetCode* targetCode)
{
    return mContext->getArgBufferSize(targetCode);
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

uint32_t MaterialManager::getMdlMaterialSize(const TargetCode* targetCode)
{
    return mContext->getMdlMaterialSize(targetCode);
}

const uint8_t* MaterialManager::getMdlMaterialData(const TargetCode* targetCode)
{
    return mContext->getMdlMaterialData(targetCode);
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
inline size_t round_to_power_of_two(size_t value, size_t power_of_two_factor)
{
    return (value + (power_of_two_factor - 1)) & ~(power_of_two_factor - 1);
}
std::vector<uint8_t> MaterialManager::Context::loadArgBlocks(const TargetCode* targetCode)
{
    std::vector<uint8_t> res;
    mi::base::Handle<Resource_callback> callback(new Resource_callback(mTransaction, targetCode->targetCode));
    for (int i = 0; i < targetCode->compiledMaterials.size(); i++)
    {
        const mi::Size argLayoutIndex = targetCode->internalsInfo[i].argument_block_index;
        if (argLayoutIndex != static_cast<mi::Size>(-1) &&
            argLayoutIndex < targetCode->targetCode->get_argument_layout_count())
        {
            // argument block for class compilation parameter data
            mi::base::Handle<const mi::neuraylib::ITarget_argument_block> arg_block;
            {
                // get the layout
                mi::base::Handle<const mi::neuraylib::ITarget_value_layout> arg_layout(
                    targetCode->targetCode->get_argument_block_layout(argLayoutIndex));

                // for the first instances of the materials, the argument block already exists
                // for further blocks new ones have to be created. To avoid special treatment,
                // an new block is created for every material
                arg_block = targetCode->targetCode->create_argument_block(
                    argLayoutIndex,
                    targetCode->compiledMaterials[i],
                    callback.get());

                if (!arg_block)
                {
                    std::cerr << ("Failed to create material argument block: ") << std::endl;
                    res.resize(4);
                    return res;
                }
            }
            // create a buffer to provide those parameters to the shader
            size_t buffer_size = round_to_power_of_two(arg_block->get_size(), 4);
            std::vector<uint8_t> argBlockData = std::vector<uint8_t>(buffer_size, 0);
            memcpy(argBlockData.data(), arg_block->get_data(), arg_block->get_size());

            res.insert(res.end(), argBlockData.begin(), argBlockData.end());
        }
    }

    if (res.empty())
    {
        res.resize(4);
    }
    return res;
};

std::vector<uint8_t> MaterialManager::Context::loadROData(const TargetCode* targetCode)
{
    std::vector<uint8_t> roData;

    size_t segCount = targetCode->targetCode->get_ro_data_segment_count();
    for (size_t i = 0; i < segCount; ++i)
    {
        const char* data = targetCode->targetCode->get_ro_data_segment_data(i);
        size_t dataSize = targetCode->targetCode->get_ro_data_segment_size(i);
        const char* name = targetCode->targetCode->get_ro_data_segment_name(i);
        std::cerr << "MDL segment [" << i << "] name : " << name << std::endl;
        
        if (dataSize != 0)
        {
            roData.insert(roData.end(), data, data + dataSize);
        }
    }

    if (roData.empty())
    {
        roData.resize(4);
    }

    return roData;
}
} // namespace nevk
