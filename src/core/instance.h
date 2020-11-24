#pragma once
#include "common.h"

class Instance
{
public:
    Instance(std::string applicationName,
             std::vector<const char*>& requiredExtensions,
             uint32_t apiVersion = VK_API_VERSION_1_0);
    ~Instance();

    VkInstance getHandle();

private:
    VkInstance handle;
};
