cmake_minimum_required(VERSION 3.18)
project(myopengl)

link_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(NAME "app")
add_executable(${NAME} src/main.cpp)
target_link_libraries(${NAME} opengl32 glfw3 glew32s)
