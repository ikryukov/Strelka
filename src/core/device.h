#pragma once

#include "common.h"
#include <vulkan/vulkan.h>

#include "core/instance.h"

class Device {

public:

	Device(Instance* instance);
	~Device();

private:

	VkPhysicalDevice GPU{ VK_NULL_HANDLE }; // Physical Device (GPU)
	VkDevice GPU_interface{ VK_NULL_HANDLE }; // Logical Device (Interface)

	VkPhysicalDeviceProperties GPU_properties{ VK_NULL_HANDLE };
	VkPhysicalDeviceFeatures GPU_features{ VK_NULL_HANDLE };

	void pickPhysicalDevice(VkInstance instance_handle);
	void createLogicalDevice();

	//-------------------------------------------------------------------
	// Necessary queue family capabilities in GPU

	VkQueue graphicsQueue{ VK_NULL_HANDLE };
	// ...

	struct QueueFamilyIndices {

		// Make variables without value at all using std::optional
		std::optional<uint32_t> graphicsFamily; // Graphical queue
		// ...

		// Condition to convinient GPU
		bool isComplete() {
			return graphicsFamily.has_value();
			// ...
		}
	} ;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice GPU);
	bool isDeviceSuitable(VkPhysicalDevice GPU);

};
