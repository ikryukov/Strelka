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
    struct Material;
    struct TargetCode;

    bool addMdlSearchPath(const char* paths[], uint32_t numPaths);

    Module* createModule(const char* file);
    Module* createMtlxModule(const char* file);
    void destroyModule(Module* module);

    Material* createMaterial(const Module* module, const char* materialName);
    void destroyMaterial(Material* material);

    const TargetCode* generateTargetCode(const std::vector<Material*>& material);
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
