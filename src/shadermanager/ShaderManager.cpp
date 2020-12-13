#include "ShaderManager.h"

using namespace nevk;

ShaderManager::ShaderManager()
{
    mSlangSession = spCreateSession(NULL);
}

ShaderManager::~ShaderManager()
{
    spDestroySession(mSlangSession);
}
