#pragma once

#include "common.h"
#include "core/instance.h"

class Device {

public:

	Device(Instance*);
	~Device();

	VkDevice getInterface();

private:

	VkPhysicalDevice           GPU;            // Physical Device
	VkDevice                   GPU_interface;  // Logical Device
	VkPhysicalDeviceProperties GPU_properties;
	VkPhysicalDeviceFeatures   GPU_features;

	// Queues
	VkQueue graphicsQueue;

	void pickPhysicalDevice(Instance*);
	void createLogicalDevice();
};
