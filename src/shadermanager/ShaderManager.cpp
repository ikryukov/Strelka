#include "ShaderManager.h"

namespace nevk
{

ShaderManager::ShaderManager()
{
    mSlangSession = spCreateSession(NULL);
}

ShaderManager::~ShaderManager()
{
    spDestroySession(mSlangSession);
}

ShaderManager::ShaderDesc ShaderManager::compileShader(const char* fileName, const char* entryPointName, Stage stage)
{
    SlangCompileRequest* slangRequest = spCreateCompileRequest(mSlangSession);

    // spSetDebugInfoLevel(slangRequest, SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SPIRV);
    SlangProfileID profileID = spFindProfile(mSlangSession, "sm_6_3");
    spSetTargetProfile(slangRequest, targetIndex, profileID);
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, fileName);

    SlangStage slangStage = SLANG_STAGE_NONE;
    switch (stage)
    {
    case nevk::ShaderManager::Stage::eVertex:
        slangStage = SLANG_STAGE_VERTEX;
        break;
    case nevk::ShaderManager::Stage::ePixel:
        slangStage = SLANG_STAGE_PIXEL;
        break;
    case nevk::ShaderManager::Stage::eCompute:
        slangStage = SLANG_STAGE_COMPUTE;
        break;
    default:
        break;
    }

    int entryPointIndex = spAddEntryPoint(slangRequest, translationUnitIndex, entryPointName, slangStage);
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
    desc.stage = stage;

    desc.code.resize(dataSize);
    memcpy(&desc.code[0], data, dataSize);
    desc.codeSize = dataSize;
    desc.type = slangStage;
    desc.slangReflection = (slang::ShaderReflection*)spGetReflection(slangRequest);
    desc.slangRequest = slangRequest;
    //spDestroyCompileRequest(slangRequest);
    return desc;
}
uint32_t ShaderManager::loadShader(const char* fileName, const char* entryPointName, Stage stage)
{
    ShaderDesc sd = compileShader(fileName, entryPointName, stage);
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
void ShaderManager::reloadAllShaders()
{
    for (const auto& shader : mShaderDescs)
    {
        loadShader(shader.fileName.c_str(), shader.entryPointName.c_str(), shader.stage);
    }
}
bool ShaderManager::getShaderCode(uint32_t id, const char*& code, uint32_t& size)
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
} // namespace nevk
