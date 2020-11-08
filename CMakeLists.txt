cmake_minimum_required(VERSION 3.18)
project(myopengl)

add_executable(${PROJECT_NAME} src/main.cpp)

# External 3rd-party libraries
    option(GLFW_BUILD_EXAMPLES OFF)
    option(GLFW_BUILD_TESTS OFF)
    option(GLFW_BUILD_DOCS OFF)
add_subdirectory(external/glfw)

target_include_directories(${PROJECT_NAME}
    PRIVATE include
    PRIVATE external/glad/include
    PRIVATE external/glfw/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE external/glad/src
    PRIVATE external/glfw/src
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32 glfw)
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} GL glfw) 
elseif(APPLE)
    #TODO
endif(WIN32)
