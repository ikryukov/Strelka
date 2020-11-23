#pragma once

#include "common.h"
#include "core/instance.h"
#include "core/device.h"

class Engine
{
public:
    Engine(GLFWwindow*);
    ~Engine();

    Instance* getInstance();
    Device* getDevice();

private:
    // External variables
    GLFWwindow* window;

    // Internal variables
    VkSurfaceKHR surface;

    // Interfaces
    std::unique_ptr<Instance> instance;
    std::unique_ptr<Device> device;

    void createInstance();
    void createDevice();
    void createSurface();
};
