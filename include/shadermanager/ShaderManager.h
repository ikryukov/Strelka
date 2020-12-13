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
        int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);
        spSetTargetProfile(slangRequest, targetIndex, spFindProfile(mSlangSession, "sm_4_0"));
        int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
        spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, fileName);
        int epIndex = spAddEntryPoint(slangRequest, translationUnitIndex, entryPointName, isPixel ? SLANG_STAGE_PIXEL : SLANG_STAGE_VERTEX);
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

        ISlangBlob* shaderBlob = nullptr;
        spGetEntryPointCodeBlob(slangRequest, epIndex, 0, &shaderBlob);

        // We extract the begin/end pointers to the output code buffers
        // using operations on the `ISlangBlob` interface.
        //
        char const* code = (char const*) shaderBlob->getBufferPointer();
        uint32_t codeSize = shaderBlob->getBufferSize();

        
        // Once we have extracted the output blobs, it is safe to destroy
        // the compile request and even the session.
        //
        spDestroyCompileRequest(slangRequest);
        return true;
    }

    SlangSession* mSlangSession = nullptr;
};
} // namespace nevk
