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
} // namespace nevk
