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


    struct ShaderDesc
    {
        std::string fileName;
        std::string entryPointName;
        bool isPixel;

        std::vector<char> code;
        uint32_t codeSize;
        SlangStage type;
    };
    std::vector<ShaderDesc> mShaderDescs;

    ShaderDesc compileShader(const char* fileName, const char* entryPointName, bool isPixel = false)
    {
        SlangCompileRequest* slangRequest = spCreateCompileRequest(mSlangSession);

        // spSetDebugInfoLevel(slangRequest, SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
        int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SPIRV);
        SlangProfileID profileID = spFindProfile(mSlangSession, "sm_6_3");
        spSetTargetProfile(slangRequest, targetIndex, profileID);
        int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
        spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, fileName);
        const SlangStage stage = isPixel ? SLANG_STAGE_FRAGMENT : SLANG_STAGE_VERTEX;
        int entryPointIndex = spAddEntryPoint(slangRequest, translationUnitIndex, entryPointName, stage);
        const SlangResult compileRes = spCompile(slangRequest);

        if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
            printf("%s\n", diagnostics);
        if (SLANG_FAILED(compileRes))
        {
            spDestroyCompileRequest(slangRequest);
            return ShaderDesc();
        }

        size_t dataSize = 0;
        void const* data = spGetEntryPointCode(slangRequest, entryPointIndex, &dataSize);
        if (!data)
            return ShaderDesc();

        ShaderDesc desc{};

        desc.fileName = std::string(fileName);
        desc.entryPointName = std::string(entryPointName);
        desc.isPixel = isPixel;

        desc.code.resize(dataSize);
        memcpy(&desc.code[0], data, dataSize);
        desc.codeSize = dataSize;
        desc.type = stage;

        spDestroyCompileRequest(slangRequest);
        return desc;
    }

    uint32_t loadShader(const char* fileName, const char* entryPointName, bool isPixel = false)
    {
        ShaderDesc sd = compileShader(fileName, entryPointName, isPixel);
        uint32_t shaderId = 0;

        // Try to reload existing shader
        for (; shaderId < mShaderDescs.size(); shaderId++)
        {
            ShaderDesc& old_sd = mShaderDescs[shaderId];
            if (old_sd.fileName == std::string(fileName) && old_sd.entryPointName == std::string(entryPointName))
            {
                old_sd.code.resize(sd.codeSize);
                memcpy(&old_sd.code[0], &sd.code[0], sd.codeSize);
                old_sd.codeSize = sd.codeSize;
                return shaderId;
            }
        }

        // Remember new data
        shaderId = mShaderDescs.size();
        mShaderDescs.push_back(sd);

        return shaderId;
    }

    void reloadAllShaders()
    {
        for (auto shader : mShaderDescs)
            loadShader(shader.fileName.c_str(), shader.entryPointName.c_str(), shader.isPixel);
    }

    bool getShaderCode(uint32_t id, const char*& code, uint32_t& size)
    {
        if (mShaderDescs.size() <= id)
        {
            return false;
        }

        ShaderDesc& sd = mShaderDescs[id];
        code = sd.code.data();
        size = sd.codeSize;

        return true;
    }

    SlangSession* mSlangSession = nullptr;
};
} // namespace nevk
