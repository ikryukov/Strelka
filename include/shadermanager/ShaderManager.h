#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include <cstdio>
#include <vector>

namespace nevk
{

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();


    struct ShaderDesc
    {
        std::vector<char> code;
        uint32_t codeSize;
        SlangStage type;
    };
    std::vector<ShaderDesc> mShaderDescs;

    uint32_t loadShader(const char* fileName, const char* entryPointName, bool isPixel = false)
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
        {
            printf("%s\n", diagnostics);
        }
        if (SLANG_FAILED(compileRes))
        {
            spDestroyCompileRequest(slangRequest);
            return -1;
        }

        size_t dataSize = 0;
        void const* data = spGetEntryPointCode(slangRequest, entryPointIndex, &dataSize);
        if (!data)
        {
            return -1;
        }

        ShaderDesc desc{};
        desc.code.resize(dataSize);
        memcpy(&desc.code[0], data, dataSize);
        desc.codeSize = dataSize;
        desc.type = stage;

        uint32_t shaderId = mShaderDescs.size();
        mShaderDescs.push_back(desc);

        spDestroyCompileRequest(slangRequest);
        return shaderId;
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
