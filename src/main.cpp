// Vulkan
#include <vulkan/vulkan.h>

// GLFW (OpenGL Framework)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLSL (OpenGL Shader Language)
    // Vertex shader - it is about calculating the shape of the figure
    // Fragment shader - it is about calculating the color of the figure
//#include "shaders.h"

#include "common.h"

//=====================================================================

#include "core/instance.h"
#include "core/device.h"

class Engine {
public:

    Engine() {
        createInstance();
        createDevice();
    }

    ~Engine() {
        delete this->instance;
        delete this->device;
    }

private:
    Instance* instance{ nullptr };
    Device* device{ nullptr };

    void createInstance() {

        // Set required extenstions
        std::vector<const char*> requiredExtensions;

            // GLFW extensions
            #ifdef _glfw3_h_
            uint32_t extensionsCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
            for (uint32_t i = 0; i < extensionsCount; i++)
                requiredExtensions.push_back(glfwExtensions[i]);
            #endif

        // Set required validations
        // ...

        // Create instance
        this->instance = new Instance("MyApp", requiredExtensions);
    }

    void createDevice() {
        this->device = new Device(this->instance);
    }

};

//=====================================================================

class Application {

public:
    static const uint32_t WIDTH = 800;
    static const uint32_t HEIGHT = 600;

    Application() {
        initWindow();
        initEngine();
        run();
        destroyEngine();
        destroyWindow();
    }

private:
    GLFWwindow* window{ nullptr };
    Engine* engine{ nullptr };

    void initWindow() {

        // GLFW: Initialize and configure
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Monitor
        GLFWmonitor* monitor = nullptr; // use glfwGetPrimaryMonitor() for full screen

        // Window
        this->window = glfwCreateWindow(WIDTH, HEIGHT, "MyOpenGL", monitor, nullptr);
        if (this->window == nullptr) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        std::cout << "Window was created successfully" << std::endl;
    }

    void initEngine() {
        this->engine = new Engine();
    }

    void run() {
        while (!glfwWindowShouldClose(this->window)) {
            glfwPollEvents(); // Check keyboard and mouse events
        }
    }

    void destroyEngine() {
        delete this->engine;
    }

    void destroyWindow() {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }
};

int main()
{
    try {
        Application app;
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
