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
    ShaderManager();
    ~ShaderManager();

    uint32_t loadShader(const char* fileName, const char* entryPointName, bool isPixel = false);
    void reloadAllShaders();
    bool getShaderCode(uint32_t id, const char*& code, uint32_t& size);

private:
    struct ShaderDesc
    {
        std::string fileName;
        std::string entryPointName;
        bool isPixel = false;
        slang::ShaderReflection* slangReflection;
        SlangCompileRequest* slangRequest;
        std::vector<char> code;
        uint32_t codeSize = 0;
        SlangStage type = SLANG_STAGE_NONE;
    };

    SlangSession* mSlangSession = nullptr;
    std::vector<ShaderDesc> mShaderDescs;

    ShaderDesc compileShader(const char* fileName, const char* entryPointName, bool isPixel = false);
};
} // namespace nevk
