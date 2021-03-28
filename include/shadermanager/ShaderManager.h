#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>

namespace nevk
{

class ShaderManager
{
public:

    enum class Stage
    {
        eVertex,
        ePixel,
        eCompute
    };

    ShaderManager();
    ~ShaderManager();

    uint32_t loadShader(const char* fileName, const char* entryPointName, Stage stage);
    void reloadAllShaders();
    bool getShaderCode(uint32_t id, const char*& code, uint32_t& size);

private:
    struct ShaderDesc
    {
        std::string fileName;
        std::string entryPointName;
        Stage stage;
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
