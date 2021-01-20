#include "window.h"

namespace nevk
{

GLFWwindow* Window::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "NeVK Example", nullptr, nullptr);
    return window;
}

} // namespace nevk