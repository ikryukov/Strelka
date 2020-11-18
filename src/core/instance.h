#pragma once

#include "common.h"
#include <vulkan/vulkan.h>

class Instance {

public:

	Instance(const std::string& applicationName,
		const std::vector<const char*>& requiredExtensions,
		uint32_t apiVersion = VK_API_VERSION_1_0);
	~Instance();

	VkInstance getHandle();

private:

	// The Vulkan instance handler
	VkInstance handle{ VK_NULL_HANDLE };
	// The enabled extensions which were both required and available
	std::vector<const char*> enabledExtensions;

};
