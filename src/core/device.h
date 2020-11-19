#pragma once

#include "common.h"
#include <vulkan/vulkan.h>

#include "core/instance.h"

class Device {

public:

	Device(Instance*);
	~Device();

private:

	VkPhysicalDevice GPU;   // Physical Device
	VkDevice GPU_interface; // Logical Device

	VkPhysicalDeviceProperties GPU_properties;
	VkPhysicalDeviceFeatures GPU_features;

	// Queue families
	VkQueue graphicsQueue;

	void pickPhysicalDevice(Instance*);
	void createLogicalDevice();
};
