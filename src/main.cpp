// GLAD (OpenGL Loader)
#include <glad/glad.h>

// GLFW (OpenGL Framework)
#include <GLFW/glfw3.h>

// GLSL (OpenGL Shader Language)
#include "shaders.h"

// Common includes
#include <iostream>

// Common constants
const GLuint WIDTH = 800, HEIGHT = 600;          // Pixs. Width and height of window
const GLfloat bg[] = { 0.2f, 0.3f, 0.3f, 1.0f }; // RGBA. Color of background

// Common functions
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main()
{
    // GLFW: Initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //==========================================================================================================================================
    // Create a GLFWwindow object which we can use for GLFW's functions

    // Monitor
    GLFWmonitor* monitor = nullptr; // use glfwGetPrimaryMonitor() for full screen

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "MyOpenGL", monitor, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //==========================================================================================================================================
    // General options

    // Load all OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "Starting GLAD context: OpenGL " << glGetString(GL_VERSION) << std::endl;

    // Set the required callback functions
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Set background color
    glClearColor(bg[0], bg[1], bg[2], bg[3]);

    //==========================================================================================================================================
    // Graphical objects

    // BUFFERS:
    // VAO - Vertex Array Object (VAO memorize VBO/EBO with attributes)
    // VBO - Vertex Buffer Objects (Array of vertices)
    // EBO - Element Buffer Objects (Array of vertices' indecies in VBO)

    // DRAW TYPES:
    // GL_STREAM_DRAW  - the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW  - the data is set only once and used many times.
    // GL_DYNAMIC_DRAW - the data is changed a lot and used many times.

    // ATTRIBUTE (arguments):
    // 1) attribute number (0, 1, 2, ...)
    // 2) number of figures (for each object)
    // 3) type of figures (GL_INT, GL_UNSIGNED_INT, GL_FLOAT, ...)
    // 4) normalization (on/off) ( try to convert figures from integer [-inf, +inf] to float [-1, 1] )
    // 5) the stride = number_of_figures * size_of_type_of_figures
    // 6) offset (set for after first attribute, nullptr for first attribute)

    // Vertices position | 0.0 - center of the screen
    GLfloat vertices[] = {
        // Position         // Colors
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom left (0)
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right (1)
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // top left (2)
         0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // top right (3)
    };
    
    // Indecies for EBO
    GLuint indecies[] = {
        0, 1, 2, // first triangle
        1, 2, 3  // second triangle
    };

    // Initialization buffers
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Configure the VAO = [VBO + EBO + attributes]
    glBindVertexArray(VAO); // Bind VAO
            
            // Set VBO - package of vertices
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // Set EBO - sequence of vertices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indecies), indecies, GL_STATIC_DRAW);

            // Set position attribute as first(0) attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(0);

            // Set color attribute as second(1) attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO

    //==========================================================================================================================================
    // Textures and Shaders

    // SHADERS:
    // Vertex shader - it is about calculating the shape ouptut of your figure
    // Fragment shader - it is about calculating the color output of your pixels

    Shader default_shader("misc/shaders/default.vs", "misc/shaders/default.frs");

    //==========================================================================================================================================
    // Game loop
    
    while (!glfwWindowShouldClose(window))
    {
        // Check keyboard and mouse events
        glfwPollEvents();

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw
        default_shader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sizeof(indecies) / sizeof(GLuint), GL_UNSIGNED_INT, nullptr);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// It's called whenever a key is pressed/released via GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        GLint status[2];
        glGetIntegerv(GL_POLYGON_MODE, status);
        if (*status == GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (*status == GL_FILL) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

// It's called whenever the window size changed
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
