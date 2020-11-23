#include "engine.h"

Engine::Engine(GLFWwindow* window) :
    window{ window },
    instance{ nullptr },
    device{ nullptr },
    surface{ VK_NULL_HANDLE }
{
    createInstance();
    createDevice();
    createSurface();
}

Engine::~Engine() {
    if (this->surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(this->instance->getHandle(), this->surface, nullptr);
}

//========================================================================================================
// Public functions

Instance* Engine::getInstance() {
    return this->instance.get();
}

Device* Engine::getDevice() {
    return this->device.get();
}

//========================================================================================================
// Private functions

void Engine::createInstance() {

    // Set required extenstions
    std::vector<const char*> requiredExtensions;

    // GLFW extensions
    uint32_t extensionsCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    for (uint32_t i = 0; i < extensionsCount; i++)
        requiredExtensions.push_back(glfwExtensions[i]);

    // Create instance
    this->instance = std::unique_ptr<Instance>( new Instance(std::string{ "MyApp" }, requiredExtensions) );
}

void Engine::createDevice() {
    this->device = std::unique_ptr<Device>( new Device(this->instance.get()) );
}

void Engine::createSurface() {
    if (glfwCreateWindowSurface(this->instance->getHandle(), window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface!");
}
