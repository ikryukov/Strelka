#pragma once

#include <memory>
#include <stdint.h>
#include <vector>

namespace nevk
{

class MaterialManager
{
    class Context;
    std::unique_ptr<Context> mContext;

public:
    struct Module;
    struct MaterialInstance;
    struct CompiledMaterial;
    struct TargetCode;
    struct TextureDescription;

    bool addMdlSearchPath(const char* paths[], uint32_t numPaths);

    Module* createModule(const char* file);
    Module* createMtlxModule(const char* file);
    void destroyModule(Module* module);

    MaterialInstance* createMaterialInstance(const Module* module, const char* materialName);
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

    const TargetCode* generateTargetCode(const std::vector<CompiledMaterial*>& material);
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

    // mtlx -> hlsl
    // std::string mtlxMaterialPath = "/Users/jswark/school/USD_Build/resources/Materials/Examples/StandardSurface/standard_surface_plastic.mtlx"; //brass_tiled.mtlx"; -- w/ images
    // std::string mtlxLibPath = "/Users/jswark/school/USD_Build/libraries";
    // mdl -> hlsl
    //std::string mIdentifier = "brushed_antique_copper"; //todo: the identifier depends on a material file // empty for mtlx mode


};
} // namespace nevk
