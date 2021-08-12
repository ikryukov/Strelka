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
    enum class Stage
    {
        eNone,
        eVertex,
        ePixel,
        eCompute
    };
    ShaderManager();
    ~ShaderManager();

    uint32_t loadShader(const char* fileName, const char* entryPointName, Stage stage);
    void reloadAllShaders();
    bool getShaderCode(uint32_t id, const char*& code, uint32_t& size);

    void printInfo(uint32_t id);
    // void printParameterBlock(slang::ShaderReflection* reflection, slang::VariableLayoutReflection* parameter);

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
};
} // namespace nevk
