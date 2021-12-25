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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>

namespace fs = std::filesystem;


namespace nevk
{
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

struct MaterialManager::TextureDescription
{
    std::string dbName;
    mi::base::Handle<const mi::neuraylib::IType_texture> textureType;
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
        configurePaths();
    };

    ~Context(){};

    // paths is array of resource pathes + mdl path
    bool addMdlSearchPath(const char* paths[], uint32_t numPaths)
    {
        mRuntime = std::make_unique<nevk::MdlRuntime>();
        if (!mRuntime->init(paths, numPaths, mPathso.c_str(), mImagePluginPath.c_str()))
        {
            return false;
        }

        mMtlxCodeGen = std::make_unique<nevk::MtlxMdlCodeGen>(mtlxLibPath.c_str());

        mMatCompiler = std::make_unique<nevk::MdlMaterialCompiler>(*mRuntime);
        mTransaction = mi::base::Handle<mi::neuraylib::ITransaction>(mRuntime->getTransaction());
        mNeuray = mRuntime->getNeuray();

        mCodeGen = std::make_unique<MdlHlslCodeGen>();
        mCodeGen->init(*mRuntime);

        return true;
    }

    Module* createMtlxModule(const char* file)
    {
        assert(mMtlxCodeGen);
        std::unique_ptr<Module> module = std::make_unique<Module>();

        bool res = mMtlxCodeGen->translate(file, mMdlSrc, module->identifier);
        if (res)
        {
            std::string mdlFile = "./misc/test_data/mtlx/" + module->identifier + ".mdl";
            std::ofstream material(mdlFile.c_str());
            material << mMdlSrc;
            material.close();
            if (!mMatCompiler->createModule(module->identifier, module->moduleName))
            {
                printf("Error: failed to create MDL module\n");
                return nullptr;
            }
            return module.release();
        }
        else
        {
            printf("Error: failed to translate MaterialX -> MDL\n");
            return nullptr;
        }        
    };

    Module* createModule(const char* file)
    {
        std::unique_ptr<Module> module = std::make_unique<Module>();

        const fs::path materialFile = file;
        module->identifier = materialFile.stem().string();

        if (!mMatCompiler->createModule(module->identifier, module->moduleName))
        {
            return nullptr;
        }

        return module.release();
    };

    void destroyModule(Module* module)
    {
        assert(module);
        delete module;
    };

    MaterialInstance* createMaterialInstance(MaterialManager::Module* module, const char* materialName)
    {
        assert(module);
        assert(materialName);

        std::unique_ptr<MaterialInstance> material = std::make_unique<MaterialInstance>();

        if (strcmp(materialName, "") == 0) // mtlx
        {
            materialName = module->identifier.c_str();
        }


        if (!mMatCompiler->createMaterialInstace(module->moduleName.c_str(), materialName, material->instance))
        {
            return nullptr;
        }

        return material.release();
    }

    void destroyMaterialInstance(MaterialInstance* matInst)
    {
        assert(matInst);
        delete matInst;
    }

    bool changeParam(MaterialInstance* matInst, ParamType type, const char* paramName, const void* paramData)
    {
        mi::base::Handle<mi::neuraylib::IValue_factory> value_factory(mMatCompiler->getFactory()->create_value_factory(mMatCompiler->getTransaction().get()));
        mi::base::Handle<mi::neuraylib::IExpression_factory> expression_factory(mMatCompiler->getFactory()->create_expression_factory(mMatCompiler->getTransaction().get()));
        mi::base::Handle<mi::neuraylib::IType_factory> type_factory(mMatCompiler->getFactory()->create_type_factory(mMatCompiler->getTransaction().get()));
        mi::base::Handle<mi::neuraylib::IValue> value;
        switch (type)
        {
        case nevk::MaterialManager::ParamType::eFloat: {
            value = value_factory->create_float(*((float*)paramData));
            break;
        }
        case nevk::MaterialManager::ParamType::eColor: {
            float* dataPtr = (float*)paramData;
            value = value_factory->create_color(dataPtr[0], dataPtr[1], dataPtr[2]);
            break;
        }
        case nevk::MaterialManager::ParamType::eTexture: {
            const TextureDescription* texDesc = (const TextureDescription*)paramData;
            value = value_factory->create_texture(texDesc->textureType.get(), texDesc->dbName.c_str());
            break;
        }
        default:
            break;
        }
        mi::base::Handle<mi::neuraylib::IExpression> expr(expression_factory->create_constant(value.get()));
        mi::Sint32 res = matInst->instance->set_argument(paramName, expr.get());
        return res == 0;
    }

    TextureDescription* createTextureDescription(const char* name, const char* gammaMode)
    {
        assert(name);
        float gamma = 1.0;
        const char* texGamma = "";
        if (strcmp(gammaMode, "srgb") == 0)
        {
            gamma = 2.2f;
            texGamma = "_srgb";
        }
        else if (strcmp(gammaMode, "linear") == 0)
        {
            gamma = 1.0f;
            texGamma = "_linear";
        }

        std::string textureDbName = std::string(name) + std::string(texGamma);
        mi::base::Handle<const mi::neuraylib::ITexture> texAccess = mi::base::Handle<const mi::neuraylib::ITexture>(
            mMatCompiler->getTransaction()->access<mi::neuraylib::ITexture>(textureDbName.c_str()));

        // check if it is in DB
        if (!texAccess.is_valid_interface())
        {
            // Load it
            mi::base::Handle<mi::neuraylib::ITexture> tex(mMatCompiler->getTransaction()->create<mi::neuraylib::ITexture>("Texture"));
            mi::base::Handle<mi::neuraylib::IImage> image(mMatCompiler->getTransaction()->create<mi::neuraylib::IImage>("Image"));
            if (image->reset_file(name) != 0)
            {
                // TODO: report error could not find texture!
                return nullptr;
            }
            std::string imageName = textureDbName + std::string("_image");
            mMatCompiler->getTransaction()->store(image.get(), imageName.c_str());

            tex->set_image(imageName.c_str());
            tex->set_gamma(gamma);
            mMatCompiler->getTransaction()->store(tex.get(), textureDbName.c_str());
        }

        mi::base::Handle<mi::neuraylib::IType_factory> typeFactory(mMatCompiler->getFactory()->create_type_factory(mMatCompiler->getTransaction().get()));
        // TODO: texture could be 1D, 3D
        mi::base::Handle<const mi::neuraylib::IType_texture> textureType(typeFactory->create_texture(mi::neuraylib::IType_texture::TS_2D));

        TextureDescription* texDesc = new TextureDescription;
        texDesc->dbName = textureDbName;
        texDesc->textureType = textureType;
        return texDesc;
    }

    const char* getTextureDbName(TextureDescription* texDesc)
    {
        return texDesc->dbName.c_str();
    }

    CompiledMaterial* compileMaterial(MaterialInstance* matInstance)
    {
        assert(matInstance);
        std::unique_ptr<CompiledMaterial> material = std::make_unique<CompiledMaterial>();
        if (!mMatCompiler->compileMaterial(matInstance->instance, material->compiledMaterial))
        {
            return nullptr;
        }

        return material.release();
    };

    void destroyCompiledMaterial(CompiledMaterial* materials)
    {
        assert(materials);
        delete materials;
    }

    const TargetCode* generateTargetCode(std::vector<CompiledMaterial*>& materials)
    {
        TargetCode* targetCode = new TargetCode;

        targetCode->mdlMaterials.resize(materials.size());

        for (uint32_t i = 0; i < materials.size(); ++i)
        {
            targetCode->compiledMaterials.push_back(materials[i]->compiledMaterial.get());
            targetCode->mdlMaterials[i].functionId = i; // TODO: ???
        }

        targetCode->targetCode = mCodeGen->translate(targetCode->compiledMaterials, targetCode->targetHlsl, targetCode->internalsInfo);
        targetCode->argBlockData = loadArgBlocks(targetCode);
        targetCode->roData = loadROData(targetCode);

        uint32_t texCount = targetCode->targetCode->get_texture_count();
        if (texCount > 0)
        {
            targetCode->resourceInfo.resize(texCount);
            for (uint32_t i = 0; i < texCount; ++i)
            {
                targetCode->resourceInfo[i].gpu_resource_array_start = i;
            }
        }
        else
        {
            targetCode->resourceInfo.resize(1);
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

    void configurePaths()
    {
        using namespace std;
        const fs::path cwd = fs::current_path();
        mtlxLibPath = "/Users/ilya/work/usd_build/libraries/";
        // mtlxLibPath = "C:/work/USD_build_debug/libraries";
        mMdlSrc = cwd.string() + "/misc/test_data/mdl/"; // path to the material

#ifdef MI_PLATFORM_WINDOWS
        mPathso = cwd.string();
        mImagePluginPath = cwd.string() + "/nv_freeimage.dll";
#else
        mPathso = cwd.string();
        mImagePluginPath = cwd.string() + "/nv_freeimage.so";
#endif
    }

    std::unique_ptr<nevk::MdlHlslCodeGen> mCodeGen = nullptr;
    std::unique_ptr<nevk::MdlMaterialCompiler> mMatCompiler = nullptr;
    std::unique_ptr<nevk::MdlRuntime> mRuntime = nullptr;
    std::unique_ptr<nevk::MtlxMdlCodeGen> mMtlxCodeGen = nullptr;

    mi::base::Handle<mi::neuraylib::ITransaction> mTransaction;
    mi::base::Handle<MdlLogger> mLogger;
    mi::base::Handle<mi::neuraylib::INeuray> mNeuray;

    std::vector<uint8_t> loadArgBlocks(TargetCode* targetCode);
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

MaterialManager::MaterialInstance* MaterialManager::createMaterialInstance(MaterialManager::Module* module, const char* materialName)
{
    return mContext->createMaterialInstance(module, materialName);
}

void MaterialManager::destroyMaterialInstance(MaterialManager::MaterialInstance* matInst)
{
    return mContext->destroyMaterialInstance(matInst);
}

MaterialManager::TextureDescription* MaterialManager::createTextureDescription(const char* name, const char* gamma)
{
    return mContext->createTextureDescription(name, gamma);
}

const char* MaterialManager::getTextureDbName(TextureDescription* texDesc)
{
    return mContext->getTextureDbName(texDesc);
}

bool MaterialManager::changeParam(MaterialManager::MaterialInstance* matInst, ParamType type, const char* paramName, const void* paramData)
{
    return mContext->changeParam(matInst, type, paramName, paramData);
}

MaterialManager::CompiledMaterial* MaterialManager::compileMaterial(MaterialManager::MaterialInstance* matInstance)
{
    return mContext->compileMaterial(matInstance);
}

void MaterialManager::destroyCompiledMaterial(MaterialManager::CompiledMaterial* material)
{
    return mContext->destroyCompiledMaterial(material);
}

const MaterialManager::TargetCode* MaterialManager::generateTargetCode(std::vector<CompiledMaterial*>& materials)
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

std::vector<uint8_t> MaterialManager::Context::loadArgBlocks(TargetCode* targetCode)
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
            // align to 4 bytes and pow of two
            size_t buffer_size = round_to_power_of_two(arg_block->get_size(), 4);
            std::vector<uint8_t> argBlockData = std::vector<uint8_t>(buffer_size, 0);
            memcpy(argBlockData.data(), arg_block->get_data(), arg_block->get_size());

            // set offset in common arg block buffer
            targetCode->mdlMaterials[i].arg_block_offset = (int)res.size();

            res.insert(res.end(), argBlockData.begin(), argBlockData.end());
        }
    }

    if (res.empty())
    {
        res.resize(4);
    }
    return res;
}

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
