#define VMA_IMPLEMENTATION

#include "common.h"
#include "engine.h"

//=====================================================================

class Application {

public:
    static const uint32_t WIDTH = 800;
    static const uint32_t HEIGHT = 600;

    Application():
        window{ nullptr },
        engine{ nullptr }
    {
        initWindow();
        initEngine();
        run();
    }

    ~Application() {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

private:
    GLFWwindow* window;
    std::unique_ptr<Engine> engine;

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
        this->engine = std::unique_ptr<Engine>(new Engine(this->window));
    }

    void run() {
        while (!glfwWindowShouldClose(this->window)) {
            glfwPollEvents(); // Check keyboard and mouse events
        }
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
