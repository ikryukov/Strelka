#pragma once
#include <slang.h>

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

private:
    SlangSession* slangSession;
};

