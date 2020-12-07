#include "ShaderManager.h"

ShaderManager::ShaderManager()
{
    this->slangSession = spCreateSession(NULL);
}

ShaderManager::~ShaderManager()
{
    spDestroySession(this->slangSession);
}
