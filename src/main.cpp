// GLFW (openGL FrameWork)
#include <GLFW/glfw3.h>

// Common includes
#include <iostream>

// Common constants
const GLuint WIDTH = 800, HEIGHT = 600;          // Pixs. Width and height of window
const GLfloat bg[] = { 0.2f, 0.3f, 0.3f, 1.0f }; // RGBA. Color of background

// Common functions
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

int main()
{
    glfwInit();

    // Version and profile of OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Make screen unresizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Monitor
    GLFWmonitor* monitor = nullptr; // use glfwGetPrimaryMonitor() for full screen

    //==========================================================================================================================================
    // Create a GLFWwindow object which we can use for GLFW's functions

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "MyOpenGL", monitor, nullptr);
    if (window == nullptr)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    printf("Starting GLFW context: OpenGL %s\n", glGetString(GL_VERSION));

    //==========================================================================================================================================
    // Common options

    // Set the required callback functions
    glfwSetKeyCallback(window, keyCallback);

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Set background color
    glClearColor(bg[0], bg[1], bg[2], bg[3]);

	//==========================================================================================================================================
    // Game loop

    while (!glfwWindowShouldClose(window))
    {
        // Check keyboard and mouse events
        glfwPollEvents();

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

