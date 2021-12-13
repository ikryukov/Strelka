#pragma once

#include <stdint.h>
#include <string>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace nevk
{
class MtlxMdlCodeGen
{
public:
    explicit MtlxMdlCodeGen(const char* mtlxlibPath);

public:
    bool translate(const char* mtlxSrc, std::string& mdlSrc, std::string& subIdentifier);

private:
    const MaterialX::FileSearchPath mMtlxlibPath;
    MaterialX::DocumentPtr mStdLib;
    MaterialX::ShaderGeneratorPtr mShaderGen;
};
}
