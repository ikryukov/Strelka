cmake_minimum_required(VERSION 3.16)
project(myopengl)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/shaders.cpp
)

##############################
# External 3rd-party libraries

# GLFW (OpenGL Framework)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
add_subdirectory(external/glfw)

##############################

target_include_directories(${PROJECT_NAME}
    PRIVATE include
    PRIVATE external/glfw/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE external/glfw/src
)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)
