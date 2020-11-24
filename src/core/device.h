#pragma once
#include "common.h"

class Device
{
public:
    Device(VkInstance, VkSurfaceKHR);
    ~Device();

    VkDevice getInterface();

private:
    // External variables
    VkInstance instance;
    VkSurfaceKHR surface;

    // Internal variables
    VkDevice GPU_interface;
    VkPhysicalDevice GPU;
    VkPhysicalDeviceProperties GPU_properties;
    VkPhysicalDeviceFeatures GPU_features;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    void pickPhysicalDevice();
    void createLogicalDevice();
};
