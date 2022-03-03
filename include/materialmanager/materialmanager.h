#pragma once

#include <memory>
#include <stdint.h>
#include <vector>
#include <string>
#include <mi/mdl_sdk.h>

namespace oka
{

class MaterialManager
{
    class Context;
    std::unique_ptr<Context> mContext;

public:
    struct Module
    {
        std::string moduleName;
        std::string identifier;
    };
    struct MaterialInstance
    {
        mi::base::Handle<mi::neuraylib::IMaterial_instance> instance;
    };

    struct CompiledMaterial
    {
        mi::base::Handle<mi::neuraylib::ICompiled_material> compiledMaterial;
    };

    struct TargetCode;
    struct TextureDescription;

    bool addMdlSearchPath(const char* paths[], uint32_t numPaths);

    Module* createModule(const char* file);
    Module* createMtlxModule(const char* file);
    void destroyModule(Module* module);

    MaterialInstance* createMaterialInstance(Module* module, const char* materialName);
    void destroyMaterialInstance(MaterialInstance* material);

    enum class ParamType: uint32_t
    {
        eFloat = 0,
        eColor,
        eTexture
    };

    bool changeParam(MaterialInstance* matInst, ParamType type, const char* paramName, const void* paramData);

    TextureDescription* createTextureDescription(const char* name, const char* gamma);
    const char* getTextureDbName(TextureDescription* texDesc);

    CompiledMaterial* compileMaterial(MaterialInstance* matInstance);
    void destroyCompiledMaterial(CompiledMaterial* compMaterial);

    const TargetCode* generateTargetCode(std::vector<CompiledMaterial*>& material);
    const char* getShaderCode(const TargetCode* targetCode);

    uint32_t getReadOnlyBlockSize(const TargetCode* targetCode);
    const uint8_t* getReadOnlyBlockData(const TargetCode* targetCode);

    uint32_t getArgBufferSize(const TargetCode* targetCode);
    const uint8_t* getArgBufferData(const TargetCode* targetCode);

    uint32_t getResourceInfoSize(const TargetCode* targetCode);
    const uint8_t* getResourceInfoData(const TargetCode* targetCode);

    uint32_t getMdlMaterialSize(const TargetCode* targetCode);
    const uint8_t* getMdlMaterialData(const TargetCode* targetCode);

    uint32_t getTextureCount(const TargetCode* targetCode);
    const char* getTextureName(const TargetCode* targetCode, uint32_t index);
    const float* getTextureData(const TargetCode* targetCode, uint32_t index);
    const char* getTextureType(const TargetCode* targetCode, uint32_t index);
    uint32_t getTextureWidth(const TargetCode* targetCode, uint32_t index);
    uint32_t getTextureHeight(const TargetCode* targetCode, uint32_t index);
    uint32_t getTextureBytesPerTexel(const TargetCode* targetCode, uint32_t index);

    MaterialManager();
    ~MaterialManager();
};
} // namespace nevk

