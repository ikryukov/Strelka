#pragma once

#include "test1.h"

namespace nevk
{
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Interface representing a desktop system based Window
class Window
{
private:
    static GLFWwindow* window;

public:
    static GLFWwindow* initWindow();
};

} // namespace nevk