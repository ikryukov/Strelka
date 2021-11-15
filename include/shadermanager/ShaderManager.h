#pragma once
#include <cstdio>
#include <iostream>
#include <slang.h>
#include <string>
#include <vector>
#include <slang-com-ptr.h>

namespace nevk
{

class ShaderManager
{
public:

    enum class ResourceType
    {
        eUnknown,
        eConstantBuffer,
        eStructuredBuffer,
        eTexture2D,
        eRWTexture2D,
        eSampler

    };

    struct ResourceDesc
    {
        std::string name;
        ResourceType type;
        bool isArray;
        uint32_t arraySize;
        uint32_t binding;
        uint32_t set;
    };

    enum class Stage: uint32_t
    {
        eNone,
        eVertex,
        ePixel,
        eCompute
    };
    ShaderManager();
    ~ShaderManager();

    uint32_t loadShader(const char* fileName, const char* entryPointName, Stage stage);
    uint32_t loadShaderFromString(const char* source, const char* entryPointName, Stage stage);
    void reloadAllShaders();
    bool getShaderCode(uint32_t id, const char*& code, uint32_t& size);

    std::vector<ResourceDesc> getResourcesDesc(uint32_t id);

private:
    struct ShaderDesc
    {
        std::string fileName;
        std::string entryPointName;
        Stage stage = Stage::eNone;
        slang::ShaderReflection* slangReflection;
        SlangCompileRequest* slangRequest;
        std::vector<char> code;
        uint32_t codeSize = 0;
        SlangStage type = SLANG_STAGE_NONE;
    };

    SlangSession* mSlangSession = nullptr;
    std::vector<ShaderDesc> mShaderDescs;

    ShaderDesc compileShader(const char* fileName, const char* entryPointName, Stage stage);
    ShaderDesc compileShaderFromString(const char* source, const char* entryPointName, Stage stage);
};
} // namespace nevk
