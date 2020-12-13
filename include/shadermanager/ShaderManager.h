#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include <cstdio>

namespace nevk
{

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    bool loadShader(const char* fileName, const char* entryPointName, bool isPixel = false)
    {
        SlangCompileRequest* slangRequest = spCreateCompileRequest(mSlangSession);

        spSetDebugInfoLevel(slangRequest, SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
        int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SPIRV);
        SlangProfileID profileID = spFindProfile(mSlangSession, "sm_6_3");
        spSetTargetProfile(slangRequest, targetIndex, profileID);
        int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
        spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, fileName);
        const SlangStage stage = isPixel ? SLANG_STAGE_FRAGMENT : SLANG_STAGE_VERTEX;
        int entryPointIndex = spAddEntryPoint(slangRequest, translationUnitIndex, entryPointName, stage);
        const SlangResult compileRes = spCompile(slangRequest);
        if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
        {
            printf("%s\n", diagnostics);
        }
        if(SLANG_FAILED(compileRes))
        {
            spDestroyCompileRequest(slangRequest);
            return false;
        }

        size_t dataSize = 0;
        void const* data = spGetEntryPointCode(slangRequest, entryPointIndex, &dataSize);
        char const* code = spGetEntryPointSource(slangRequest, entryPointIndex);
        if (!data)
        {
            return false;
        }
        
        spDestroyCompileRequest(slangRequest);
        return true;
    }

    SlangSession* mSlangSession = nullptr;
};
} // namespace nevk
